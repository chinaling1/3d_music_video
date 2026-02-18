/**
 * @file project_manager.cpp
 * @brief 项目管理器实现
 */

#include "project_manager.h"
#include "../../src/core/logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <random>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
namespace v3d {
namespace app {

using json = nlohmann::json;

ProjectManager::ProjectManager()
    : projectOpen_(false)
    , modified_(false)
    , autoSaveEnabled_(true)
    , autoSaveInterval_(300)
    , running_(true)
{
    settings_.frameRate = 30;
    settings_.totalFrames = 100;
    settings_.width = 1920;
    settings_.height = 1080;
    settings_.renderPreset = "default";
    
    autoSaveThread_ = std::thread(&ProjectManager::autoSaveThread, this);
}

ProjectManager::~ProjectManager() {
    running_ = false;
    if (autoSaveThread_.joinable()) {
        autoSaveThread_.join();
    }
    
    if (projectOpen_) {
        closeProject();
    }
}

ProjectManager& ProjectManager::getInstance() {
    static ProjectManager instance;
    return instance;
}

bool ProjectManager::newProject(const std::string& name, const std::string& path) {
    if (projectOpen_) {
        closeProject();
    }
    
    projectName_ = name;
    projectPath_ = path;
    projectFile_ = path + "/" + name + ".v3dproj";
    
    createProjectStructure(path);
    
    assets_.clear();
    scenes_.clear();
    versions_.clear();
    modified_ = false;
    projectOpen_ = true;
    
    createVersion("Initial version");
    
    return saveProjectFile(projectFile_);
}

bool ProjectManager::openProject(const std::string& filePath) {
    if (projectOpen_) {
        closeProject();
    }
    
    projectFile_ = filePath;
    
    if (!loadProjectFile(filePath)) {
        return false;
    }
    
    fs::path p(filePath);
    projectPath_ = p.parent_path().string();
    projectName_ = p.stem().string();
    
    projectOpen_ = true;
    modified_ = false;
    
    return true;
}

bool ProjectManager::saveProject() {
    if (!projectOpen_) return false;
    return saveProjectFile(projectFile_);
}

bool ProjectManager::saveProjectAs(const std::string& filePath) {
    if (!projectOpen_) return false;
    
    projectFile_ = filePath;
    fs::path p(filePath);
    projectPath_ = p.parent_path().string();
    projectName_ = p.stem().string();
    
    return saveProjectFile(filePath);
}

bool ProjectManager::closeProject() {
    if (!projectOpen_) return true;
    
    if (modified_) {
        performAutoSave();
    }
    
    projectOpen_ = false;
    projectName_.clear();
    projectPath_.clear();
    projectFile_.clear();
    assets_.clear();
    scenes_.clear();
    modified_ = false;
    
    return true;
}

void ProjectManager::createProjectStructure(const std::string& path) {
    fs::create_directories(path);
    fs::create_directories(path + "/assets");
    fs::create_directories(path + "/assets/models");
    fs::create_directories(path + "/assets/textures");
    fs::create_directories(path + "/assets/audio");
    fs::create_directories(path + "/assets/animations");
    fs::create_directories(path + "/scenes");
    fs::create_directories(path + "/versions");
    fs::create_directories(path + "/thumbnails");
    fs::create_directories(path + "/cache");
}

bool ProjectManager::saveProjectFile(const std::string& filePath) {
    json j;
    
    j["name"] = projectName_;
    j["version"] = "1.0";
    
    json settingsJson;
    settingsJson["frameRate"] = settings_.frameRate;
    settingsJson["totalFrames"] = settings_.totalFrames;
    settingsJson["width"] = settings_.width;
    settingsJson["height"] = settings_.height;
    settingsJson["renderPreset"] = settings_.renderPreset;
    j["settings"] = settingsJson;
    
    json assetsJson = json::array();
    for (const auto& [id, asset] : assets_) {
        json assetJson;
        assetJson["id"] = asset.id;
        assetJson["name"] = asset.name;
        assetJson["type"] = asset.type;
        assetJson["path"] = asset.path;
        assetJson["thumbnailPath"] = asset.thumbnailPath;
        assetsJson.push_back(assetJson);
    }
    j["assets"] = assetsJson;
    
    json scenesJson = json::array();
    for (const auto& [id, scene] : scenes_) {
        json sceneJson;
        sceneJson["id"] = scene.id;
        sceneJson["name"] = scene.name;
        sceneJson["objectIds"] = scene.objectIds;
        scenesJson.push_back(sceneJson);
    }
    j["scenes"] = scenesJson;
    
    std::ofstream file(filePath);
    if (!file.is_open()) return false;
    
    file << j.dump(4);
    file.close();
    
    modified_ = false;
    return true;
}

bool ProjectManager::loadProjectFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    try {
        json j = json::parse(buffer.str());
        
        projectName_ = j["name"].get<std::string>();
        
        if (j.contains("settings")) {
            auto& s = j["settings"];
            settings_.frameRate = s["frameRate"].get<int>();
            settings_.totalFrames = s["totalFrames"].get<int>();
            settings_.width = s["width"].get<int>();
            settings_.height = s["height"].get<int>();
            settings_.renderPreset = s["renderPreset"].get<std::string>();
        }
        
        if (j.contains("assets")) {
            for (const auto& assetJson : j["assets"]) {
                ProjectAsset asset;
                asset.id = assetJson["id"].get<std::string>();
                asset.name = assetJson["name"].get<std::string>();
                asset.type = assetJson["type"].get<std::string>();
                asset.path = assetJson["path"].get<std::string>();
                if (assetJson.contains("thumbnailPath")) {
                    asset.thumbnailPath = assetJson["thumbnailPath"].get<std::string>();
                }
                assets_[asset.id] = asset;
            }
        }
        
        if (j.contains("scenes")) {
            for (const auto& sceneJson : j["scenes"]) {
                ProjectScene scene;
                scene.id = sceneJson["id"].get<std::string>();
                scene.name = sceneJson["name"].get<std::string>();
                scene.objectIds = sceneJson["objectIds"].get<std::vector<std::string>>();
                scenes_[scene.id] = scene;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void ProjectManager::setAutoSaveEnabled(bool enabled) {
    autoSaveEnabled_ = enabled;
}

void ProjectManager::setAutoSaveInterval(int seconds) {
    autoSaveInterval_ = seconds;
}

void ProjectManager::autoSaveThread() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        if (autoSaveEnabled_ && projectOpen_ && modified_) {
            static auto lastSave = std::chrono::system_clock::now();
            auto now = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSave).count();
            
            if (elapsed >= autoSaveInterval_) {
                performAutoSave();
                lastSave = now;
            }
        }
    }
}

void ProjectManager::performAutoSave() {
    if (!projectOpen_) return;
    
    std::string versionId = createVersion("Auto-save", true);
    
    if (progressCallback_) {
        progressCallback_(1.0f, "Auto-saved");
    }
}

std::string ProjectManager::createVersion(const std::string& description, bool isAutoSave) {
    std::string versionId = generateUniqueId();
    
    ProjectVersion version;
    version.versionId = versionId;
    version.description = description;
    version.timestamp = std::chrono::system_clock::now();
    version.isAutoSave = isAutoSave;
    
    std::string versionDir = getVersionDirectory();
    fs::create_directories(versionDir + "/" + versionId);
    
    copyProjectToVersion(versionId);
    
    version.fileSize = 0;
    for (const auto& entry : fs::recursive_directory_iterator(versionDir + "/" + versionId)) {
        if (entry.is_regular_file()) {
            version.fileSize += entry.file_size();
        }
    }
    
    versions_.push_back(version);
    
    if (versionCallback_) {
        versionCallback_(version);
    }
    
    return versionId;
}

bool ProjectManager::restoreVersion(const std::string& versionId) {
    std::string versionDir = getVersionDirectory() + "/" + versionId;
    
    if (!fs::exists(versionDir)) return false;
    
    std::string versionProjFile = versionDir + "/" + projectName_ + ".v3dproj";
    
    return loadProjectFile(versionProjFile);
}

bool ProjectManager::deleteVersion(const std::string& versionId) {
    std::string versionDir = getVersionDirectory() + "/" + versionId;
    
    if (!fs::exists(versionDir)) return false;
    
    fs::remove_all(versionDir);
    
    versions_.erase(
        std::remove_if(versions_.begin(), versions_.end(),
            [&versionId](const ProjectVersion& v) { return v.versionId == versionId; }),
        versions_.end()
    );
    
    return true;
}

std::vector<ProjectVersion> ProjectManager::getVersionHistory() const {
    return versions_;
}

ProjectVersion ProjectManager::getVersion(const std::string& versionId) const {
    for (const auto& v : versions_) {
        if (v.versionId == versionId) return v;
    }
    return ProjectVersion();
}

bool ProjectManager::hasVersion(const std::string& versionId) const {
    for (const auto& v : versions_) {
        if (v.versionId == versionId) return true;
    }
    return false;
}

std::string ProjectManager::getPreviewForVersion(const std::string& versionId) {
    std::string versionDir = getVersionDirectory() + "/" + versionId;
    std::string previewPath = versionDir + "/preview.png";
    
    if (fs::exists(previewPath)) {
        return previewPath;
    }
    
    return "";
}

bool ProjectManager::generateVersionPreview(const std::string& versionId) {
    return true;
}

std::string ProjectManager::getVersionDirectory() const {
    return projectPath_ + "/versions";
}

bool ProjectManager::copyProjectToVersion(const std::string& versionId) {
    std::string versionDir = getVersionDirectory() + "/" + versionId;
    
    try {
        fs::copy(projectFile_, versionDir + "/" + projectName_ + ".v3dproj",
                 fs::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        return false;
    }
}

std::string ProjectManager::generateUniqueId() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";
    
    std::string uuid;
    for (int i = 0; i < 32; i++) {
        uuid += hex[dis(gen)];
        if (i == 7 || i == 11 || i == 15 || i == 19) {
            uuid += '-';
        }
    }
    
    return uuid;
}

void ProjectManager::addAsset(const ProjectAsset& asset) {
    assets_[asset.id] = asset;
    modified_ = true;
}

void ProjectManager::removeAsset(const std::string& assetId) {
    assets_.erase(assetId);
    modified_ = true;
}

ProjectAsset ProjectManager::getAsset(const std::string& assetId) const {
    auto it = assets_.find(assetId);
    if (it != assets_.end()) return it->second;
    return ProjectAsset();
}

std::vector<ProjectAsset> ProjectManager::getAllAssets() const {
    std::vector<ProjectAsset> result;
    for (const auto& [id, asset] : assets_) {
        result.push_back(asset);
    }
    return result;
}

std::vector<ProjectAsset> ProjectManager::getAssetsByType(const std::string& type) const {
    std::vector<ProjectAsset> result;
    for (const auto& [id, asset] : assets_) {
        if (asset.type == type) result.push_back(asset);
    }
    return result;
}

void ProjectManager::addScene(const ProjectScene& scene) {
    scenes_[scene.id] = scene;
    modified_ = true;
}

void ProjectManager::removeScene(const std::string& sceneId) {
    scenes_.erase(sceneId);
    modified_ = true;
}

ProjectScene ProjectManager::getScene(const std::string& sceneId) const {
    auto it = scenes_.find(sceneId);
    if (it != scenes_.end()) return it->second;
    return ProjectScene();
}

std::vector<ProjectScene> ProjectManager::getAllScenes() const {
    std::vector<ProjectScene> result;
    for (const auto& [id, scene] : scenes_) {
        result.push_back(scene);
    }
    return result;
}

void ProjectManager::setSettings(const ProjectSettings& settings) {
    settings_ = settings;
    modified_ = true;
}

void ProjectManager::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

void ProjectManager::setVersionCallback(VersionCallback callback) {
    versionCallback_ = callback;
}

void ProjectManager::setModified(bool modified) {
    modified_ = modified;
}

bool ProjectManager::exportProject(const std::string& exportPath, const std::string& format) {
    return false;
}

bool ProjectManager::importAssets(const std::vector<std::string>& filePaths) {
    for (const auto& filePath : filePaths) {
        fs::path p(filePath);
        std::string ext = p.extension().string();
        std::string type;
        
        if (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb") {
            type = "model";
        } else if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".exr") {
            type = "texture";
        } else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
            type = "audio";
        } else {
            continue;
        }
        
        ProjectAsset asset;
        asset.id = generateUniqueId();
        asset.name = p.stem().string();
        asset.type = type;
        asset.path = filePath;
        asset.lastModified = std::chrono::system_clock::now();
        
        std::string destPath = projectPath_ + "/assets/" + type + "s/" + p.filename().string();
        fs::copy_file(filePath, destPath, fs::copy_options::overwrite_existing);
        asset.path = destPath;
        
        addAsset(asset);
    }
    
    return true;
}

std::string ProjectManager::generateThumbnail(const std::string& assetId) {
    return "";
}

void ProjectManager::undo() {
    v3d::core::CommandManager::getInstance().undo();
}

void ProjectManager::redo() {
    v3d::core::CommandManager::getInstance().redo();
}

bool ProjectManager::canUndo() const {
    return v3d::core::CommandManager::getInstance().canUndo();
}

bool ProjectManager::canRedo() const {
    return v3d::core::CommandManager::getInstance().canRedo();
}

std::string ProjectManager::getUndoDescription() const {
    return v3d::core::CommandManager::getInstance().getUndoDescription();
}

std::string ProjectManager::getRedoDescription() const {
    return v3d::core::CommandManager::getInstance().getRedoDescription();
}

}
}
