/**
 * @file file_loader.h
 * @brief File Loader - Provides file loading and management functionality
 * 
 * This module implements a complete file handling system, including:
 * - FileFormat: File format enumeration
 * - FileMetadata: File metadata structure
 * - FileLoader: Synchronous file loader
 * - AsyncFileLoader: Asynchronous file loader
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>

namespace v3d {
namespace io {

enum class FileFormat {
    Unknown,
    OBJ,
    FBX,
    GLTF,
    GLB,
    DAE,
    STL,
    PLY,
    PMX,
    VMD,
    BVH,
    WAV,
    MP3,
    OGG,
    FLAC,
    MID,
    PNG,
    JPG,
    EXR,
    HDR,
    TGA,
    BMP
};

struct FileMetadata {
    std::string path;
    std::string name;
    size_t size;
    FileFormat format;
    bool compressed;
    std::unordered_map<std::string, std::string> properties;
};

class FileLoader {
public:
    static FileFormat detectFormat(const std::string& filePath);
    static std::string getFormatExtension(FileFormat format);
    static bool exists(const std::string& filePath);
    static size_t getFileSize(const std::string& filePath);
    static bool isDirectory(const std::string& path);
    static std::vector<uint8_t> readBinaryFile(const std::string& filePath);
    static std::string readTextFile(const std::string& filePath);
    static bool writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data);
    static bool writeTextFile(const std::string& filePath, const std::string& content);
    static std::vector<std::string> listFiles(const std::string& directory, const std::string& extension = "");
    static std::vector<std::string> listDirectories(const std::string& directory);
    static bool createDirectory(const std::string& path);
    static bool deleteFile(const std::string& filePath);
    static bool deleteDirectory(const std::string& path);
    static std::string getAbsolutePath(const std::string& path);
    static std::string getRelativePath(const std::string& path, const std::string& base);
    static std::string getFileName(const std::string& path);
    static std::string getFileExtension(const std::string& path);
    static std::string getDirectoryPath(const std::string& path);
    static std::string joinPath(const std::string& base, const std::string& relative);
    static std::string normalizePath(const std::string& path);
    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

class AsyncFileLoader {
public:
    AsyncFileLoader();
    ~AsyncFileLoader();

    void loadFile(const std::string& filePath, std::function<void(const std::vector<uint8_t>&)> callback);
    void loadTextFile(const std::string& filePath, std::function<void(const std::string&)> callback);
    void loadMultipleFiles(const std::vector<std::string>& filePaths,
                          std::function<void(const std::unordered_map<std::string, std::vector<uint8_t>>&)> callback);
    void cancel();
    float getProgress() const;
    bool isLoading() const;
    bool isComplete() const;
    void setMaxConcurrentLoads(int maxLoads);
    int getMaxConcurrentLoads() const;

private:
    void workerThread();

    std::thread workerThread_;
    std::mutex mutex_;
    std::condition_variable condition_;

    struct LoadTask {
        std::string filePath;
        std::function<void(const std::vector<uint8_t>&)> callback;
    };

    std::deque<LoadTask> taskQueue_;
    std::atomic<bool> running_;
    std::atomic<bool> cancelled_;
    std::atomic<float> progress_;
    std::atomic<int> maxConcurrentLoads_;
    std::atomic<int> activeLoads_;
};

}
}
