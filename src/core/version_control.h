/**
 * @file version_control.h
 * @brief Version Control System - Provides undo/redo functionality, change tracking and version management
 * 
 * This module implements a complete version control system, including:
 * - Command pattern: Supports undo/redo operations
 * - ChangeTracker: Records all change history
 * - VersionManager: Manages version tree and version rollback
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <stack>
#include <string>
#include <chrono>

namespace v3d {
namespace core {

/**
 * @class Command
 * @brief Command base class - Core interface for command pattern implementation
 * 
 * All undoable operations should inherit from this class and implement
 * the corresponding virtual functions.
 */
class Command {
public:
    virtual ~Command() = default;
    
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual std::string getDescription() const = 0;
    virtual size_t getMemorySize() const = 0;
    virtual bool isMergeableWith(const Command* other) const { return false; }
    virtual void mergeWith(Command* other) {}
    virtual Command* clone() const = 0;
};

/**
 * @class CommandManager
 * @brief Command Manager - Manages command execution, undo and redo
 */
class CommandManager {
public:
    static CommandManager& getInstance();
    
    void executeCommand(std::unique_ptr<Command> command);
    void undo();
    void redo();
    void clear();
    bool canUndo() const;
    bool canRedo() const;
    std::string getUndoDescription() const;
    std::string getRedoDescription() const;
    size_t getUndoStackSize() const;
    size_t getRedoStackSize() const;
    size_t getTotalMemoryUsage() const;
    void setMaxMemorySize(size_t maxBytes);
    void setMaxUndoSteps(size_t maxSteps);
    void saveCheckpoint(const std::string& name);
    void loadCheckpoint(const std::string& name);
    void clearCheckpoints();
    std::vector<std::string> getCheckpointNames() const;

private:
    CommandManager();
    ~CommandManager();
    void trimMemory();
    void trimSteps();

    std::stack<std::unique_ptr<Command>> undoStack_;
    std::stack<std::unique_ptr<Command>> redoStack_;
    std::vector<std::pair<std::string, std::vector<std::unique_ptr<Command>>>> checkpoints_;
    size_t maxMemorySize_;
    size_t maxUndoSteps_;
    size_t currentMemoryUsage_;
};

/**
 * @class ChangeTracker
 * @brief Change Tracker - Records all property change history
 */
class ChangeTracker {
public:
    struct ChangeRecord {
        std::string objectId;
        std::string propertyName;
        std::string oldValue;
        std::string newValue;
        std::chrono::system_clock::time_point timestamp;
        std::string userId;
        std::string sessionId;
    };

    static ChangeTracker& getInstance();
    void recordChange(const ChangeRecord& record);
    std::vector<ChangeRecord> getChangeHistory(const std::string& objectId = "") const;
    std::vector<ChangeRecord> getChangesSince(const std::chrono::system_clock::time_point& time) const;
    void exportChanges(const std::string& filePath) const;
    void importChanges(const std::string& filePath);
    void clear();

private:
    ChangeTracker();
    ~ChangeTracker();
    std::vector<ChangeRecord> changeHistory_;
};

/**
 * @class VersionManager
 * @brief Version Manager - Manages version tree and version rollback
 */
class VersionManager {
public:
    struct VersionInfo {
        std::string versionId;
        std::string description;
        std::chrono::system_clock::time_point timestamp;
        std::string userId;
        std::string parentId;
        std::vector<std::string> childIds;
        std::string checksum;
        size_t size;
    };

    static VersionManager& getInstance();
    std::string createVersion(const std::string& description, const std::string& parentId = "");
    bool revertToVersion(const std::string& versionId);
    bool deleteVersion(const std::string& versionId);
    VersionInfo getVersionInfo(const std::string& versionId) const;
    std::vector<VersionInfo> getVersionHistory() const;
    std::vector<VersionInfo> getVersionBranch(const std::string& versionId) const;
    std::string getCurrentVersion() const;
    std::string getLatestVersion() const;
    bool compareVersions(const std::string& versionId1, const std::string& versionId2,
                         std::vector<std::string>& differences) const;
    void exportVersion(const std::string& versionId, const std::string& filePath) const;
    std::string importVersion(const std::string& filePath);
    void setMaxVersions(size_t maxVersions);
    void setAutoSaveInterval(int seconds);

private:
    VersionManager();
    ~VersionManager();
    std::string generateVersionId() const;
    std::string calculateChecksum(const std::string& versionId) const;
    void autoSave();

    std::vector<VersionInfo> versions_;
    std::string currentVersion_;
    size_t maxVersions_;
    int autoSaveInterval_;
};

}
}
