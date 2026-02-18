/**
 * @file project_manager.h
 * @brief 项目管理器 - 处理项目文件操作、自动保存和版本管理
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include "../../src/core/version_control.h"

namespace v3d {
namespace app {

struct ProjectAsset {
    std::string id;
    std::string name;
    std::string type;
    std::string path;
    std::string thumbnailPath;
    std::chrono::system_clock::time_point lastModified;
};

struct ProjectScene {
    std::string id;
    std::string name;
    std::vector<std::string> objectIds;
    std::chrono::system_clock::time_point lastModified;
};

struct ProjectVersion {
    std::string versionId;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    std::string thumbnailPath;
    size_t fileSize;
    bool isAutoSave;
};

struct ProjectSettings {
    int frameRate;
    int totalFrames;
    int width;
    int height;
    std::string renderPreset;
    std::unordered_map<std::string, std::string> customSettings;
};

class ProjectManager {
public:
    using ProgressCallback = std::function<void(float, const std::string&)>;
    using VersionCallback = std::function<void(const ProjectVersion&)>;

    ProjectManager();
    ~ProjectManager();
    
    static ProjectManager& getInstance();
    
    bool newProject(const std::string& name, const std::string& path);
    bool openProject(const std::string& filePath);
    bool saveProject();
    bool saveProjectAs(const std::string& filePath);
    bool closeProject();
    
    bool isProjectOpen() const { return projectOpen_; }
    std::string getProjectName() const { return projectName_; }
    std::string getProjectPath() const { return projectPath_; }
    std::string getProjectFile() const { return projectFile_; }
    
    void setAutoSaveEnabled(bool enabled);
    bool isAutoSaveEnabled() const { return autoSaveEnabled_; }
    void setAutoSaveInterval(int seconds);
    int getAutoSaveInterval() const { return autoSaveInterval_; }
    
    std::string createVersion(const std::string& description, bool isAutoSave = false);
    bool restoreVersion(const std::string& versionId);
    bool deleteVersion(const std::string& versionId);
    std::vector<ProjectVersion> getVersionHistory() const;
    ProjectVersion getVersion(const std::string& versionId) const;
    bool hasVersion(const std::string& versionId) const;
    
    std::string getPreviewForVersion(const std::string& versionId);
    bool generateVersionPreview(const std::string& versionId);
    
    void addAsset(const ProjectAsset& asset);
    void removeAsset(const std::string& assetId);
    ProjectAsset getAsset(const std::string& assetId) const;
    std::vector<ProjectAsset> getAllAssets() const;
    std::vector<ProjectAsset> getAssetsByType(const std::string& type) const;
    
    void addScene(const ProjectScene& scene);
    void removeScene(const std::string& sceneId);
    ProjectScene getScene(const std::string& sceneId) const;
    std::vector<ProjectScene> getAllScenes() const;
    
    ProjectSettings& getSettings() { return settings_; }
    const ProjectSettings& getSettings() const { return settings_; }
    void setSettings(const ProjectSettings& settings);
    
    void setProgressCallback(ProgressCallback callback);
    void setVersionCallback(VersionCallback callback);
    
    void setModified(bool modified);
    bool isModified() const { return modified_; }
    
    bool exportProject(const std::string& exportPath, const std::string& format);
    bool importAssets(const std::vector<std::string>& filePaths);
    
    std::string generateThumbnail(const std::string& assetId);
    
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    std::string getUndoDescription() const;
    std::string getRedoDescription() const;

private:
    void autoSaveThread();
    void performAutoSave();
    bool saveProjectFile(const std::string& filePath);
    bool loadProjectFile(const std::string& filePath);
    void createProjectStructure(const std::string& path);
    std::string generateUniqueId() const;
    std::string getVersionDirectory() const;
    bool copyProjectToVersion(const std::string& versionId);
    
    bool projectOpen_;
    std::string projectName_;
    std::string projectPath_;
    std::string projectFile_;
    
    std::unordered_map<std::string, ProjectAsset> assets_;
    std::unordered_map<std::string, ProjectScene> scenes_;
    std::vector<ProjectVersion> versions_;
    ProjectSettings settings_;
    
    bool modified_;
    
    std::thread autoSaveThread_;
    std::mutex mutex_;
    std::atomic<bool> autoSaveEnabled_;
    std::atomic<int> autoSaveInterval_;
    std::atomic<bool> running_;
    
    ProgressCallback progressCallback_;
    VersionCallback versionCallback_;
    
    std::string currentVersionId_;
};

}
}
