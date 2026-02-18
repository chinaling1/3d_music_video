#include "asset_manager.h"
#include <algorithm>
#include <random>
#include <thread>
#include <future>

namespace v3d {
namespace io {

Asset::Asset(const std::string& id, AssetType type)
    : id_(id)
    , type_(type)
    , state_(AssetLoadState::Unloaded)
    , referenceCount_(0)
    , lastAccessed_(std::chrono::system_clock::now()) {
    metadata_.id = id;
    metadata_.type = type;
    metadata_.state = state_;
    metadata_.referenceCount = 0;
}

Asset::~Asset() {
}

void Asset::setLastAccessed() {
    lastAccessed_ = std::chrono::system_clock::now();
    metadata_.lastAccessed = lastAccessed_;
}

MeshAsset::MeshAsset(const std::string& id)
    : Asset(id, AssetType::Mesh) {
}

MeshAsset::~MeshAsset() {
    unload();
}

void MeshAsset::setMesh(std::shared_ptr<modeling::Mesh> mesh) {
    mesh_ = mesh;
    setState(AssetLoadState::Loaded);
    metadata_.memoryUsage = getMemoryUsage();
}

size_t MeshAsset::getMemoryUsage() const {
    if (!mesh_) return 0;
    size_t usage = sizeof(MeshAsset);
    usage += mesh_->getVertices().size() * sizeof(modeling::Vertex);
    usage += mesh_->getFaces().size() * sizeof(modeling::Face);
    return usage;
}

void MeshAsset::unload() {
    mesh_.reset();
    setState(AssetLoadState::Unloaded);
    metadata_.memoryUsage = 0;
}

SkeletonAsset::SkeletonAsset(const std::string& id)
    : Asset(id, AssetType::Skeleton) {
}

SkeletonAsset::~SkeletonAsset() {
    unload();
}

void SkeletonAsset::setSkeleton(std::shared_ptr<animation::Skeleton> skeleton) {
    skeleton_ = skeleton;
    setState(AssetLoadState::Loaded);
    metadata_.memoryUsage = getMemoryUsage();
}

size_t SkeletonAsset::getMemoryUsage() const {
    if (!skeleton_) return 0;
    return sizeof(SkeletonAsset) + skeleton_->getBoneCount() * sizeof(animation::Bone);
}

void SkeletonAsset::unload() {
    skeleton_.reset();
    setState(AssetLoadState::Unloaded);
    metadata_.memoryUsage = 0;
}

AnimationAsset::AnimationAsset(const std::string& id)
    : Asset(id, AssetType::Animation) {
}

AnimationAsset::~AnimationAsset() {
    unload();
}

void AnimationAsset::setAnimationClip(std::shared_ptr<animation::AnimationClip> clip) {
    clip_ = clip;
    setState(AssetLoadState::Loaded);
    metadata_.memoryUsage = getMemoryUsage();
}

size_t AnimationAsset::getMemoryUsage() const {
    if (!clip_) return 0;
    return sizeof(AnimationAsset) + clip_->getDuration() * 1024;
}

void AnimationAsset::unload() {
    clip_.reset();
    setState(AssetLoadState::Unloaded);
    metadata_.memoryUsage = 0;
}

AudioAsset::AudioAsset(const std::string& id)
    : Asset(id, AssetType::Audio) {
}

AudioAsset::~AudioAsset() {
    unload();
}

void AudioAsset::setAudioBuffer(std::shared_ptr<audio::AudioBuffer> buffer) {
    buffer_ = buffer;
    setState(AssetLoadState::Loaded);
    metadata_.memoryUsage = getMemoryUsage();
}

size_t AudioAsset::getMemoryUsage() const {
    if (!buffer_) return 0;
    return sizeof(AudioAsset) + buffer_->getSizeInBytes();
}

void AudioAsset::unload() {
    buffer_.reset();
    setState(AssetLoadState::Unloaded);
    metadata_.memoryUsage = 0;
}

ImageAsset::ImageAsset(const std::string& id)
    : Asset(id, AssetType::Image)
    , width_(0)
    , height_(0)
    , channels_(0) {
}

ImageAsset::~ImageAsset() {
    unload();
}

void ImageAsset::setImageData(int width, int height, int channels, const std::vector<uint8_t>& data) {
    width_ = width;
    height_ = height;
    channels_ = channels;
    data_ = data;
    setState(AssetLoadState::Loaded);
    metadata_.memoryUsage = getMemoryUsage();
}

size_t ImageAsset::getMemoryUsage() const {
    return sizeof(ImageAsset) + data_.size();
}

void ImageAsset::unload() {
    data_.clear();
    data_.shrink_to_fit();
    width_ = 0;
    height_ = 0;
    channels_ = 0;
    setState(AssetLoadState::Unloaded);
    metadata_.memoryUsage = 0;
}

MaterialAsset::MaterialAsset(const std::string& id)
    : Asset(id, AssetType::Material) {
}

MaterialAsset::~MaterialAsset() {
    unload();
}

void MaterialAsset::setMaterial(std::shared_ptr<modeling::Material> material) {
    material_ = material;
    setState(AssetLoadState::Loaded);
    metadata_.memoryUsage = getMemoryUsage();
}

size_t MaterialAsset::getMemoryUsage() const {
    return sizeof(MaterialAsset);
}

void MaterialAsset::unload() {
    material_.reset();
    setState(AssetLoadState::Unloaded);
    metadata_.memoryUsage = 0;
}

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

AssetManager::AssetManager()
    : totalMemoryUsage_(0) {
}

AssetManager::~AssetManager() {
    cleanup();
}

AssetHandle AssetManager::loadAsset(const std::string& path, AssetType type) {
    std::string resolvedPath = resolveAssetPath(path);
    std::string assetId = generateAssetId(resolvedPath);

    {
        std::lock_guard<std::mutex> lock(assetsMutex_);

        auto it = assets_.find(assetId);
        if (it != assets_.end()) {
            if (it->second->getState() == AssetLoadState::Loaded) {
                it->second->incrementRefCount();
                it->second->setLastAccessed();
                AssetHandle handle;
                handle.id = assetId;
                handle.type = it->second->getType();
                return handle;
            }
        }
    }

    auto asset = createAsset(type, assetId);
    if (!asset) {
        AssetHandle handle;
        return handle;
    }

    asset->setState(AssetLoadState::Loading);

    if (progressCallback_) {
        progressCallback_(resolvedPath, 0.0f);
    }

    bool loaded = loadAssetData(asset, resolvedPath);

    if (loaded) {
        asset->setState(AssetLoadState::Loaded);
        asset->incrementRefCount();
        asset->setLastAccessed();

        {
            std::lock_guard<std::mutex> lock(assetsMutex_);
            assets_[assetId] = asset;
            totalMemoryUsage_ += asset->getMemoryUsage();
        }

        checkMemoryUsage();

        if (progressCallback_) {
            progressCallback_(resolvedPath, 1.0f);
        }

        AssetHandle handle;
        handle.id = assetId;
        handle.type = type;
        return handle;
    } else {
        asset->setState(AssetLoadState::Failed);
        if (errorCallback_) {
            errorCallback_(resolvedPath, asset->getMetadata().errorMessage);
        }
        AssetHandle handle;
        return handle;
    }
}

AssetHandle AssetManager::loadAssetAsync(const std::string& path, AssetType type,
                                         std::function<void(const AssetHandle&)> callback) {
    std::string resolvedPath = resolveAssetPath(path);
    std::string assetId = generateAssetId(resolvedPath);

    {
        std::lock_guard<std::mutex> lock(assetsMutex_);

        auto it = assets_.find(assetId);
        if (it != assets_.end() && it->second->getState() == AssetLoadState::Loaded) {
            it->second->incrementRefCount();
            it->second->setLastAccessed();
            AssetHandle handle;
            handle.id = assetId;
            handle.type = it->second->getType();
            if (callback) callback(handle);
            return handle;
        }
    }

    std::thread([this, resolvedPath, type, assetId, callback]() {
        auto handle = loadAsset(resolvedPath, type);
        if (callback) callback(handle);
    }).detach();

    AssetHandle handle;
    handle.id = assetId;
    handle.type = type;
    return handle;
}

void AssetManager::unloadAsset(const AssetHandle& handle) {
    if (!handle.isValid()) return;

    std::lock_guard<std::mutex> lock(assetsMutex_);

    auto it = assets_.find(handle.id);
    if (it != assets_.end()) {
        it->second->decrementRefCount();

        if (it->second->getRefCount() <= 0 && cacheConfig_.enableAutoUnload) {
            size_t memoryUsage = it->second->getMemoryUsage();
            it->second->unload();
            totalMemoryUsage_ -= memoryUsage;
            assets_.erase(it);
        }
    }
}

void AssetManager::unloadAllAssets() {
    std::lock_guard<std::mutex> lock(assetsMutex_);

    for (auto& pair : assets_) {
        pair.second->unload();
    }

    assets_.clear();
    totalMemoryUsage_ = 0;
}

void AssetManager::unloadUnusedAssets() {
    std::lock_guard<std::mutex> lock(assetsMutex_);

    for (auto it = assets_.begin(); it != assets_.end(); ) {
        if (it->second->getRefCount() <= 0) {
            size_t memoryUsage = it->second->getMemoryUsage();
            it->second->unload();
            totalMemoryUsage_ -= memoryUsage;
            it = assets_.erase(it);
        } else {
            ++it;
        }
    }
}

bool AssetManager::isAssetLoaded(const AssetHandle& handle) const {
    if (!handle.isValid()) return false;

    std::lock_guard<std::mutex> lock(assetsMutex_);

    auto it = assets_.find(handle.id);
    return it != assets_.end() && it->second->getState() == AssetLoadState::Loaded;
}

AssetLoadState AssetManager::getAssetState(const AssetHandle& handle) const {
    if (!handle.isValid()) return AssetLoadState::Unloaded;

    std::lock_guard<std::mutex> lock(assetsMutex_);

    auto it = assets_.find(handle.id);
    if (it != assets_.end()) {
        return it->second->getState();
    }

    return AssetLoadState::Unloaded;
}

AssetMetadata AssetManager::getAssetMetadata(const AssetHandle& handle) const {
    AssetMetadata metadata;

    if (!handle.isValid()) return metadata;

    std::lock_guard<std::mutex> lock(assetsMutex_);

    auto it = assets_.find(handle.id);
    if (it != assets_.end()) {
        metadata = it->second->getMetadata();
    }

    return metadata;
}

std::vector<AssetMetadata> AssetManager::getAllAssetsMetadata() const {
    std::vector<AssetMetadata> metadataList;

    std::lock_guard<std::mutex> lock(assetsMutex_);

    metadataList.reserve(assets_.size());
    for (const auto& pair : assets_) {
        metadataList.push_back(pair.second->getMetadata());
    }

    return metadataList;
}

size_t AssetManager::getTotalMemoryUsage() const {
    return totalMemoryUsage_;
}

size_t AssetManager::getLoadedAssetCount() const {
    std::lock_guard<std::mutex> lock(assetsMutex_);
    return assets_.size();
}

void AssetManager::setCacheConfig(const AssetCacheConfig& config) {
    cacheConfig_ = config;
    checkMemoryUsage();
}

const AssetCacheConfig& AssetManager::getCacheConfig() const {
    return cacheConfig_;
}

void AssetManager::registerAssetPath(const std::string& alias, const std::string& path) {
    std::lock_guard<std::mutex> lock(assetsMutex_);
    assetPathAliases_[alias] = path;
}

std::string AssetManager::resolveAssetPath(const std::string& path) const {
    std::string resolvedPath = path;

    for (const auto& pair : assetPathAliases_) {
        if (path.find(pair.first + "/") == 0) {
            resolvedPath = pair.second + path.substr(pair.first.length());
            break;
        }
    }

    return FileLoader::normalizePath(resolvedPath);
}

void AssetManager::setProgressCallback(std::function<void(const std::string&, float)> callback) {
    progressCallback_ = callback;
}

void AssetManager::setErrorCallback(std::function<void(const std::string&, const std::string&)> callback) {
    errorCallback_ = callback;
}

void AssetManager::update() {
    if (!cacheConfig_.enableAutoUnload) return;

    auto now = std::chrono::system_clock::now();
    std::vector<std::string> assetsToUnload;

    {
        std::lock_guard<std::mutex> lock(assetsMutex_);

        for (const auto& pair : assets_) {
            if (pair.second->getRefCount() <= 0) {
                auto idleTime = std::chrono::duration_cast<std::chrono::seconds>(
                    now - pair.second->getLastAccessed()).count();

                if (idleTime >= cacheConfig_.maxIdleTime.count()) {
                    assetsToUnload.push_back(pair.first);
                }
            }
        }
    }

    for (const auto& assetId : assetsToUnload) {
        AssetHandle handle;
        handle.id = assetId;
        unloadAsset(handle);
    }
}

void AssetManager::cleanup() {
    unloadAllAssets();
}

std::string AssetManager::generateAssetId(const std::string& path) const {
    std::string id = path;
    std::replace(id.begin(), id.end(), '\\', '/');
    std::replace(id.begin(), id.end(), ':', '_');
    std::replace(id.begin(), id.end(), '.', '_');

    std::hash<std::string> hasher;
    size_t hash = hasher(path);

    return id + "_" + std::to_string(hash);
}

AssetType AssetManager::detectAssetType(const std::string& path) const {
    FileFormat format = FileLoader::detectFormat(path);

    switch (format) {
        case FileFormat::OBJ:
        case FileFormat::FBX:
        case FileFormat::GLTF:
        case FileFormat::GLB:
        case FileFormat::DAE:
        case FileFormat::STL:
        case FileFormat::PLY:
        case FileFormat::PMX:
            return AssetType::Mesh;

        case FileFormat::VMD:
        case FileFormat::BVH:
            return AssetType::Animation;

        case FileFormat::WAV:
        case FileFormat::MP3:
        case FileFormat::OGG:
        case FileFormat::FLAC:
            return AssetType::Audio;

        case FileFormat::PNG:
        case FileFormat::JPG:
        case FileFormat::EXR:
        case FileFormat::HDR:
        case FileFormat::TGA:
        case FileFormat::BMP:
            return AssetType::Image;

        default:
            return AssetType::Unknown;
    }
}

std::shared_ptr<Asset> AssetManager::createAsset(AssetType type, const std::string& id) {
    switch (type) {
        case AssetType::Mesh:
            return std::make_shared<MeshAsset>(id);
        case AssetType::Skeleton:
            return std::make_shared<SkeletonAsset>(id);
        case AssetType::Animation:
            return std::make_shared<AnimationAsset>(id);
        case AssetType::Audio:
            return std::make_shared<AudioAsset>(id);
        case AssetType::Image:
            return std::make_shared<ImageAsset>(id);
        case AssetType::Material:
            return std::make_shared<MaterialAsset>(id);
        default:
            return nullptr;
    }
}

bool AssetManager::loadAssetData(std::shared_ptr<Asset> asset, const std::string& path) {
    try {
        switch (asset->getType()) {
            case AssetType::Mesh: {
                auto meshAsset = std::dynamic_pointer_cast<MeshAsset>(asset);
                auto mesh = MeshLoader::load(path);
                if (mesh) {
                    meshAsset->setMesh(mesh);
                    return true;
                }
                break;
            }
            case AssetType::Skeleton: {
                auto skeletonAsset = std::dynamic_pointer_cast<SkeletonAsset>(asset);
                auto skeleton = AnimationLoader::loadSkeleton(path);
                if (skeleton) {
                    skeletonAsset->setSkeleton(skeleton);
                    return true;
                }
                break;
            }
            case AssetType::Animation: {
                auto animationAsset = std::dynamic_pointer_cast<AnimationAsset>(asset);
                auto clip = AnimationLoader::loadAnimationClip(path);
                if (clip) {
                    animationAsset->setAnimationClip(clip);
                    return true;
                }
                break;
            }
            case AssetType::Audio: {
                auto audioAsset = std::dynamic_pointer_cast<AudioAsset>(asset);
                auto buffer = AudioLoader::load(path);
                if (buffer) {
                    audioAsset->setAudioBuffer(buffer);
                    return true;
                }
                break;
            }
            case AssetType::Image: {
                auto imageAsset = std::dynamic_pointer_cast<ImageAsset>(asset);
                int width, height, channels;
                auto data = ImageLoader::load(path, width, height, channels);
                if (!data.empty()) {
                    imageAsset->setImageData(width, height, channels, data);
                    return true;
                }
                break;
            }
            default:
                break;
        }
    } catch (const std::exception& e) {
        AssetMetadata metadata = asset->getMetadata();
        metadata.errorMessage = e.what();
        asset->setMetadata(metadata);
    }

    return false;
}

void AssetManager::checkMemoryUsage() {
    if (!cacheConfig_.enableAutoUnload) return;

    if (totalMemoryUsage_ > cacheConfig_.maxMemoryUsage * cacheConfig_.unloadThreshold) {
        unloadOldestAssets(static_cast<size_t>(cacheConfig_.maxMemoryUsage * cacheConfig_.unloadThreshold));
    }
}

void AssetManager::unloadOldestAssets(size_t targetMemoryUsage) {
    std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> assetTimestamps;

    {
        std::lock_guard<std::mutex> lock(assetsMutex_);

        for (const auto& pair : assets_) {
            if (pair.second->getRefCount() <= 0) {
                assetTimestamps.push_back({pair.first, pair.second->getLastAccessed()});
            }
        }
    }

    std::sort(assetTimestamps.begin(), assetTimestamps.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    for (const auto& pair : assetTimestamps) {
        if (totalMemoryUsage_ <= targetMemoryUsage) break;

        AssetHandle handle;
        handle.id = pair.first;
        unloadAsset(handle);
    }
}

}
}
