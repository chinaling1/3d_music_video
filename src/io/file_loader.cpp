#include "file_loader.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <unordered_map>

namespace v3d {
namespace io {

std::function<void(float)> FileLoader::progressCallback_;
std::function<void(const std::string&)> FileLoader::errorCallback_;

FileFormat FileLoader::detectFormat(const std::string& filePath) {
    std::string ext = getFileExtension(filePath);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    static const std::unordered_map<std::string, FileFormat> formatMap = {
        {".obj", FileFormat::OBJ},
        {".fbx", FileFormat::FBX},
        {".gltf", FileFormat::GLTF},
        {".glb", FileFormat::GLB},
        {".dae", FileFormat::DAE},
        {".stl", FileFormat::STL},
        {".ply", FileFormat::PLY},
        {".pmx", FileFormat::PMX},
        {".vmd", FileFormat::VMD},
        {".bvh", FileFormat::BVH},
        {".wav", FileFormat::WAV},
        {".mp3", FileFormat::MP3},
        {".ogg", FileFormat::OGG},
        {".flac", FileFormat::FLAC},
        {".mid", FileFormat::MID},
        {".midi", FileFormat::MID},
        {".png", FileFormat::PNG},
        {".jpg", FileFormat::JPG},
        {".jpeg", FileFormat::JPG},
        {".exr", FileFormat::EXR},
        {".hdr", FileFormat::HDR},
        {".tga", FileFormat::TGA},
        {".bmp", FileFormat::BMP}
    };

    auto it = formatMap.find(ext);
    return it != formatMap.end() ? it->second : FileFormat::Unknown;
}

std::string FileLoader::getFormatExtension(FileFormat format) {
    switch (format) {
        case FileFormat::OBJ: return ".obj";
        case FileFormat::FBX: return ".fbx";
        case FileFormat::GLTF: return ".gltf";
        case FileFormat::GLB: return ".glb";
        case FileFormat::DAE: return ".dae";
        case FileFormat::STL: return ".stl";
        case FileFormat::PLY: return ".ply";
        case FileFormat::PMX: return ".pmx";
        case FileFormat::VMD: return ".vmd";
        case FileFormat::BVH: return ".bvh";
        case FileFormat::WAV: return ".wav";
        case FileFormat::MP3: return ".mp3";
        case FileFormat::OGG: return ".ogg";
        case FileFormat::FLAC: return ".flac";
        case FileFormat::MID: return ".mid";
        case FileFormat::PNG: return ".png";
        case FileFormat::JPG: return ".jpg";
        case FileFormat::EXR: return ".exr";
        case FileFormat::HDR: return ".hdr";
        case FileFormat::TGA: return ".tga";
        case FileFormat::BMP: return ".bmp";
        default: return "";
    }
}

bool FileLoader::exists(const std::string& filePath) {
    return std::filesystem::exists(filePath);
}

size_t FileLoader::getFileSize(const std::string& filePath) {
    try {
        return std::filesystem::file_size(filePath);
    } catch (...) {
        return 0;
    }
}

bool FileLoader::isDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

std::vector<uint8_t> FileLoader::readBinaryFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to open file: " + filePath);
        }
        return {};
    }

    size_t fileSize = file.tellg();
    std::vector<uint8_t> data(fileSize);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return data;
}

std::string FileLoader::readTextFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to open file: " + filePath);
        }
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return content;
}

bool FileLoader::writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create file: " + filePath);
        }
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool FileLoader::writeTextFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create file: " + filePath);
        }
        return false;
    }

    file << content;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

std::vector<std::string> FileLoader::listFiles(const std::string& directory, const std::string& extension) {
    std::vector<std::string> files;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                if (extension.empty() || getFileExtension(path) == extension) {
                    files.push_back(path);
                }
            }
        }
    } catch (...) {
        if (errorCallback_) {
            errorCallback_("Failed to list files in directory: " + directory);
        }
    }

    return files;
}

std::vector<std::string> FileLoader::listDirectories(const std::string& directory) {
    std::vector<std::string> directories;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().string());
            }
        }
    } catch (...) {
        if (errorCallback_) {
            errorCallback_("Failed to list directories: " + directory);
        }
    }

    return directories;
}

bool FileLoader::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (...) {
        if (errorCallback_) {
            errorCallback_("Failed to create directory: " + path);
        }
        return false;
    }
}

bool FileLoader::deleteFile(const std::string& filePath) {
    try {
        return std::filesystem::remove(filePath);
    } catch (...) {
        if (errorCallback_) {
            errorCallback_("Failed to delete file: " + filePath);
        }
        return false;
    }
}

bool FileLoader::deleteDirectory(const std::string& path) {
    try {
        return std::filesystem::remove_all(path);
    } catch (...) {
        if (errorCallback_) {
            errorCallback_("Failed to delete directory: " + path);
        }
        return false;
    }
}

std::string FileLoader::getAbsolutePath(const std::string& path) {
    try {
        return std::filesystem::absolute(path).string();
    } catch (...) {
        return path;
    }
}

std::string FileLoader::getRelativePath(const std::string& path, const std::string& base) {
    try {
        return std::filesystem::relative(path, base).string();
    } catch (...) {
        return path;
    }
}

std::string FileLoader::getFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

std::string FileLoader::getFileExtension(const std::string& path) {
    return std::filesystem::path(path).extension().string();
}

std::string FileLoader::getDirectoryPath(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

std::string FileLoader::joinPath(const std::string& base, const std::string& relative) {
    return (std::filesystem::path(base) / relative).string();
}

std::string FileLoader::normalizePath(const std::string& path) {
    try {
        return std::filesystem::path(path).lexically_normal().string();
    } catch (...) {
        return path;
    }
}

void FileLoader::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void FileLoader::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

AsyncFileLoader::AsyncFileLoader()
    : running_(false)
    , cancelled_(false)
    , progress_(0.0f)
    , maxConcurrentLoads_(4)
    , activeLoads_(0) {
}

AsyncFileLoader::~AsyncFileLoader() {
    cancel();
}

void AsyncFileLoader::loadFile(const std::string& filePath, std::function<void(const std::vector<uint8_t>&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    LoadTask task;
    task.filePath = filePath;
    task.callback = callback;
    taskQueue_.push_back(task);

    if (!running_) {
        running_ = true;
        workerThread_ = std::thread(&AsyncFileLoader::workerThread, this);
    }

    condition_.notify_one();
}

void AsyncFileLoader::loadTextFile(const std::string& filePath, std::function<void(const std::string&)> callback) {
    loadFile(filePath, [callback](const std::vector<uint8_t>& data) {
        std::string text(data.begin(), data.end());
        callback(text);
    });
}

void AsyncFileLoader::loadMultipleFiles(const std::vector<std::string>& filePaths,
                                       std::function<void(const std::unordered_map<std::string, std::vector<uint8_t>>&)> callback) {
    std::unordered_map<std::string, std::vector<uint8_t>> results;
    std::mutex resultsMutex;
    int completed = 0;
    std::condition_variable doneCondition;
    size_t totalFiles = filePaths.size();

    for (const auto& filePath : filePaths) {
        loadFile(filePath, [&results, &resultsMutex, &completed, &doneCondition, totalFiles, filePath]
            (const std::vector<uint8_t>& data) {
            std::lock_guard<std::mutex> lock(resultsMutex);
            results[filePath] = data;
            completed++;

            if (static_cast<size_t>(completed) >= totalFiles) {
                doneCondition.notify_one();
            }
        });
    }

    std::unique_lock<std::mutex> lock(resultsMutex);
    doneCondition.wait(lock, [&completed, totalFiles] {
        return static_cast<size_t>(completed) >= totalFiles;
    });

    callback(results);
}

void AsyncFileLoader::cancel() {
    cancelled_ = true;
    condition_.notify_all();

    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    running_ = false;
    taskQueue_.clear();
    progress_ = 0.0f;
    cancelled_ = false;
}

float AsyncFileLoader::getProgress() const {
    return progress_.load();
}

bool AsyncFileLoader::isLoading() const {
    return running_ && !taskQueue_.empty();
}

bool AsyncFileLoader::isComplete() const {
    return !running_ && taskQueue_.empty();
}

void AsyncFileLoader::setMaxConcurrentLoads(int maxLoads) {
    maxConcurrentLoads_ = std::max(1, maxLoads);
}

int AsyncFileLoader::getMaxConcurrentLoads() const {
    return maxConcurrentLoads_;
}

void AsyncFileLoader::workerThread() {
    while (running_ && !cancelled_) {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_.wait(lock, [this] {
            return !taskQueue_.empty() || cancelled_;
        });

        if (cancelled_) {
            break;
        }

        if (taskQueue_.empty()) {
            running_ = false;
            break;
        }

        while (activeLoads_ < maxConcurrentLoads_ && !taskQueue_.empty() && !cancelled_) {
            LoadTask task = taskQueue_.front();
            taskQueue_.pop_front();
            activeLoads_++;
            lock.unlock();

            std::vector<uint8_t> data = FileLoader::readBinaryFile(task.filePath);

            if (task.callback) {
                task.callback(data);
            }

            lock.lock();
            activeLoads_--;

            progress_ = 1.0f - static_cast<float>(taskQueue_.size()) /
                                 static_cast<float>(taskQueue_.size() + activeLoads_);
        }
    }
}

}
}