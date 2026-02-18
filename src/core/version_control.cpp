#include "version_control.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>
#include <fstream>
#include <nlohmann/json.hpp>

namespace v3d {
namespace core {

CommandManager& CommandManager::getInstance() {
    static CommandManager instance;
    return instance;
}

CommandManager::CommandManager()
    : maxMemorySize_(512 * 1024 * 1024)
    , maxUndoSteps_(100)
    , currentMemoryUsage_(0) {
}

CommandManager::~CommandManager() {
    clear();
}

void CommandManager::executeCommand(std::unique_ptr<Command> command) {
    command->execute();

    if (!redoStack_.empty()) {
        while (!redoStack_.empty()) {
            currentMemoryUsage_ -= redoStack_.top()->getMemorySize();
            redoStack_.pop();
        }
    }

    if (!undoStack_.empty() && command->isMergeableWith(undoStack_.top().get())) {
        undoStack_.top()->mergeWith(command.get());
    } else {
        currentMemoryUsage_ += command->getMemorySize();
        undoStack_.push(std::move(command));
    }

    trimMemory();
    trimSteps();
}

void CommandManager::undo() {
    if (undoStack_.empty()) return;

    auto command = std::move(undoStack_.top());
    undoStack_.pop();
    currentMemoryUsage_ -= command->getMemorySize();

    command->undo();
    currentMemoryUsage_ += command->getMemorySize();
    redoStack_.push(std::move(command));
}

void CommandManager::redo() {
    if (redoStack_.empty()) return;

    auto command = std::move(redoStack_.top());
    redoStack_.pop();
    currentMemoryUsage_ -= command->getMemorySize();

    command->redo();
    currentMemoryUsage_ += command->getMemorySize();
    undoStack_.push(std::move(command));
}

void CommandManager::clear() {
    while (!undoStack_.empty()) {
        currentMemoryUsage_ -= undoStack_.top()->getMemorySize();
        undoStack_.pop();
    }
    while (!redoStack_.empty()) {
        currentMemoryUsage_ -= redoStack_.top()->getMemorySize();
        redoStack_.pop();
    }
    checkpoints_.clear();
}

bool CommandManager::canUndo() const {
    return !undoStack_.empty();
}

bool CommandManager::canRedo() const {
    return !redoStack_.empty();
}

std::string CommandManager::getUndoDescription() const {
    return undoStack_.empty() ? "" : undoStack_.top()->getDescription();
}

std::string CommandManager::getRedoDescription() const {
    return redoStack_.empty() ? "" : redoStack_.top()->getDescription();
}

size_t CommandManager::getUndoStackSize() const {
    return undoStack_.size();
}

size_t CommandManager::getRedoStackSize() const {
    return redoStack_.size();
}

size_t CommandManager::getTotalMemoryUsage() const {
    return currentMemoryUsage_;
}

void CommandManager::setMaxMemorySize(size_t maxBytes) {
    maxMemorySize_ = maxBytes;
    trimMemory();
}

void CommandManager::setMaxUndoSteps(size_t maxSteps) {
    maxUndoSteps_ = maxSteps;
    trimSteps();
}

void CommandManager::saveCheckpoint(const std::string& name) {
    std::vector<std::unique_ptr<Command>> checkpointCommands;

    while (!undoStack_.empty()) {
        checkpointCommands.push_back(std::move(undoStack_.top()));
        undoStack_.pop();
    }

    std::reverse(checkpointCommands.begin(), checkpointCommands.end());
    checkpoints_.emplace_back(name, std::move(checkpointCommands));
}

void CommandManager::loadCheckpoint(const std::string& name) {
    auto it = std::find_if(checkpoints_.begin(), checkpoints_.end(),
        [&name](const auto& pair) { return pair.first == name; });

    if (it != checkpoints_.end()) {
        clear();
        for (const auto& command : it->second) {
            executeCommand(std::unique_ptr<Command>(command->clone()));
        }
    }
}

void CommandManager::clearCheckpoints() {
    checkpoints_.clear();
}

std::vector<std::string> CommandManager::getCheckpointNames() const {
    std::vector<std::string> names;
    for (const auto& pair : checkpoints_) {
        names.push_back(pair.first);
    }
    return names;
}

void CommandManager::trimMemory() {
    while (currentMemoryUsage_ > maxMemorySize_ && !undoStack_.empty()) {
        currentMemoryUsage_ -= undoStack_.top()->getMemorySize();
        undoStack_.pop();
    }
}

void CommandManager::trimSteps() {
    while (undoStack_.size() > maxUndoSteps_ && !undoStack_.empty()) {
        currentMemoryUsage_ -= undoStack_.top()->getMemorySize();
        undoStack_.pop();
    }
}

ChangeTracker& ChangeTracker::getInstance() {
    static ChangeTracker instance;
    return instance;
}

ChangeTracker::ChangeTracker() {
}

ChangeTracker::~ChangeTracker() {
}

void ChangeTracker::recordChange(const ChangeRecord& record) {
    changeHistory_.push_back(record);
}

std::vector<ChangeTracker::ChangeRecord> ChangeTracker::getChangeHistory(
    const std::string& objectId) const {

    if (objectId.empty()) {
        return changeHistory_;
    }

    std::vector<ChangeRecord> filtered;
    for (const auto& record : changeHistory_) {
        if (record.objectId == objectId) {
            filtered.push_back(record);
        }
    }
    return filtered;
}

std::vector<ChangeTracker::ChangeRecord> ChangeTracker::getChangesSince(
    const std::chrono::system_clock::time_point& time) const {

    std::vector<ChangeRecord> filtered;
    for (const auto& record : changeHistory_) {
        if (record.timestamp >= time) {
            filtered.push_back(record);
        }
    }
    return filtered;
}

void ChangeTracker::exportChanges(const std::string& filePath) const {
    nlohmann::json j;
    for (const auto& record : changeHistory_) {
        j.push_back({
            {"objectId", record.objectId},
            {"propertyName", record.propertyName},
            {"oldValue", record.oldValue},
            {"newValue", record.newValue},
            {"timestamp", std::chrono::system_clock::to_time_t(record.timestamp)},
            {"userId", record.userId},
            {"sessionId", record.sessionId}
        });
    }

    std::ofstream file(filePath);
    file << j.dump(2);
}

void ChangeTracker::importChanges(const std::string& filePath) {
    std::ifstream file(filePath);
    nlohmann::json j;
    file >> j;

    for (const auto& item : j) {
        ChangeRecord record;
        record.objectId = item["objectId"];
        record.propertyName = item["propertyName"];
        record.oldValue = item["oldValue"];
        record.newValue = item["newValue"];
        record.timestamp = std::chrono::system_clock::from_time_t(item["timestamp"]);
        record.userId = item["userId"];
        record.sessionId = item["sessionId"];
        changeHistory_.push_back(record);
    }
}

void ChangeTracker::clear() {
    changeHistory_.clear();
}

VersionManager& VersionManager::getInstance() {
    static VersionManager instance;
    return instance;
}

VersionManager::VersionManager()
    : maxVersions_(50)
    , autoSaveInterval_(300) {
}

VersionManager::~VersionManager() {
}

std::string VersionManager::createVersion(const std::string& description, const std::string& parentId) {
    VersionInfo info;
    info.versionId = generateVersionId();
    info.description = description;
    info.timestamp = std::chrono::system_clock::now();
    info.userId = "system";
    info.parentId = parentId;
    info.checksum = calculateChecksum(info.versionId);
    info.size = 0;

    if (!parentId.empty()) {
        auto it = std::find_if(versions_.begin(), versions_.end(),
            [&parentId](const auto& v) { return v.versionId == parentId; });
        if (it != versions_.end()) {
            it->childIds.push_back(info.versionId);
        }
    }

    versions_.push_back(info);
    currentVersion_ = info.versionId;

    if (versions_.size() > maxVersions_) {
        versions_.erase(versions_.begin());
    }

    return info.versionId;
}

bool VersionManager::revertToVersion(const std::string& versionId) {
    auto it = std::find_if(versions_.begin(), versions_.end(),
        [&versionId](const auto& v) { return v.versionId == versionId; });

    if (it != versions_.end()) {
        currentVersion_ = versionId;
        return true;
    }
    return false;
}

bool VersionManager::deleteVersion(const std::string& versionId) {
    auto it = std::find_if(versions_.begin(), versions_.end(),
        [&versionId](const auto& v) { return v.versionId == versionId; });

    if (it != versions_.end()) {
        versions_.erase(it);
        return true;
    }
    return false;
}

VersionManager::VersionInfo VersionManager::getVersionInfo(const std::string& versionId) const {
    auto it = std::find_if(versions_.begin(), versions_.end(),
        [&versionId](const auto& v) { return v.versionId == versionId; });

    if (it != versions_.end()) {
        return *it;
    }
    return VersionInfo{};
}

std::vector<VersionManager::VersionInfo> VersionManager::getVersionHistory() const {
    return versions_;
}

std::vector<VersionManager::VersionInfo> VersionManager::getVersionBranch(const std::string& versionId) const {
    std::vector<VersionInfo> branch;
    auto it = std::find_if(versions_.begin(), versions_.end(),
        [&versionId](const auto& v) { return v.versionId == versionId; });

    if (it != versions_.end()) {
        branch.push_back(*it);
        for (const auto& childId : it->childIds) {
            auto childBranch = getVersionBranch(childId);
            branch.insert(branch.end(), childBranch.begin(), childBranch.end());
        }
    }
    return branch;
}

std::string VersionManager::getCurrentVersion() const {
    return currentVersion_;
}

std::string VersionManager::getLatestVersion() const {
    return versions_.empty() ? "" : versions_.back().versionId;
}

bool VersionManager::compareVersions(const std::string& versionId1, const std::string& versionId2,
                                     std::vector<std::string>& differences) const {
    auto info1 = getVersionInfo(versionId1);
    auto info2 = getVersionInfo(versionId2);

    if (info1.checksum != info2.checksum) {
        differences.push_back("Checksum mismatch");
        return false;
    }

    return true;
}

void VersionManager::exportVersion(const std::string& versionId, const std::string& filePath) const {
    auto info = getVersionInfo(versionId);

    nlohmann::json j;
    j["versionId"] = info.versionId;
    j["description"] = info.description;
    j["timestamp"] = std::chrono::system_clock::to_time_t(info.timestamp);
    j["userId"] = info.userId;
    j["parentId"] = info.parentId;
    j["checksum"] = info.checksum;
    j["size"] = info.size;

    std::ofstream file(filePath);
    file << j.dump(2);
}

std::string VersionManager::importVersion(const std::string& filePath) {
    std::ifstream file(filePath);
    nlohmann::json j;
    file >> j;

    VersionInfo info;
    info.versionId = j["versionId"];
    info.description = j["description"];
    info.timestamp = std::chrono::system_clock::from_time_t(j["timestamp"]);
    info.userId = j["userId"];
    info.parentId = j["parentId"];
    info.checksum = j["checksum"];
    info.size = j["size"];

    versions_.push_back(info);
    return info.versionId;
}

void VersionManager::setMaxVersions(size_t maxVersions) {
    maxVersions_ = maxVersions;
}

void VersionManager::setAutoSaveInterval(int seconds) {
    autoSaveInterval_ = seconds;
}

std::string VersionManager::generateVersionId() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < 8; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 4; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-4";

    for (int i = 0; i < 3; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    ss << std::setw(1) << (8 + dis(gen) % 4);

    for (int i = 0; i < 3; ++i) {
        ss << std::setw(1) << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 12; ++i) {
        ss << std::setw(1) << dis(gen);
    }

    return ss.str();
}

std::string VersionManager::calculateChecksum(const std::string& versionId) const {
    std::hash<std::string> hasher;
    return std::to_string(hasher(versionId));
}

void VersionManager::autoSave() {
}

}
}