#include <gtest/gtest.h>
#include "../../src/core/version_control.h"
#include "../utils/test_utils.h"

using namespace v3d::core;
using namespace v3d::test;

class TestCommand : public Command {
public:
    TestCommand(int* value, int newValue, const std::string& description)
        : value_(value)
        , newValue_(newValue)
        , oldValue_(*value)
        , description_(description) {
    }

    void execute() override {
        *value_ = newValue_;
    }

    void undo() override {
        *value_ = oldValue_;
    }

    void redo() override {
        *value_ = newValue_;
    }

    std::string getDescription() const override {
        return description_;
    }

    size_t getMemorySize() const override {
        return sizeof(TestCommand);
    }

    Command* clone() const override {
        return new TestCommand(value_, newValue_, description_);
    }

private:
    int* value_;
    int newValue_;
    int oldValue_;
    std::string description_;
};

class VersionControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        commandManager_ = &CommandManager::getInstance();
        changeTracker_ = &ChangeTracker::getInstance();
        versionManager_ = &VersionManager::getInstance();
        
        testValue_ = 0;
    }

    void TearDown() override {
        commandManager_->clear();
        changeTracker_->clear();
    }

    CommandManager* commandManager_;
    ChangeTracker* changeTracker_;
    VersionManager* versionManager_;
    
    int testValue_;
};

TEST_F(VersionControlTest, CommandManager_ExecuteCommand) {
    auto cmd = std::make_unique<TestCommand>(&testValue_, 42, "Set value to 42");
    
    EXPECT_EQ(testValue_, 0);
    
    commandManager_->executeCommand(std::move(cmd));
    
    EXPECT_EQ(testValue_, 42);
    EXPECT_TRUE(commandManager_->canUndo());
    EXPECT_FALSE(commandManager_->canRedo());
}

TEST_F(VersionControlTest, CommandManager_UndoRedo) {
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 10, "Set to 10"));
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 20, "Set to 20"));
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 30, "Set to 30"));
    
    EXPECT_EQ(testValue_, 30);
    EXPECT_EQ(commandManager_->getUndoStackSize(), 3);
    EXPECT_EQ(commandManager_->getRedoStackSize(), 0);
    
    commandManager_->undo();
    EXPECT_EQ(testValue_, 20);
    EXPECT_EQ(commandManager_->getUndoStackSize(), 2);
    EXPECT_EQ(commandManager_->getRedoStackSize(), 1);
    
    commandManager_->undo();
    EXPECT_EQ(testValue_, 10);
    EXPECT_EQ(commandManager_->getUndoStackSize(), 1);
    EXPECT_EQ(commandManager_->getRedoStackSize(), 2);
    
    commandManager_->redo();
    EXPECT_EQ(testValue_, 20);
    EXPECT_EQ(commandManager_->getUndoStackSize(), 2);
    EXPECT_EQ(commandManager_->getRedoStackSize(), 1);
}

TEST_F(VersionControlTest, CommandManager_Checkpoint) {
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 10, "Set to 10"));
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 20, "Set to 20"));
    
    commandManager_->saveCheckpoint("Checkpoint1");
    
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 30, "Set to 30"));
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 40, "Set to 40"));
    
    EXPECT_EQ(testValue_, 40);
    
    commandManager_->loadCheckpoint("Checkpoint1");
    
    EXPECT_EQ(testValue_, 20);
    EXPECT_TRUE(commandManager_->canUndo());
    EXPECT_FALSE(commandManager_->canRedo());
}

TEST_F(VersionControlTest, CommandManager_MemoryLimit) {
    commandManager_->setMaxMemorySize(1024);
    
    for (int i = 0; i < 100; ++i) {
        commandManager_->executeCommand(
            std::make_unique<TestCommand>(&testValue_, i, "Command " + std::to_string(i))
        );
    }
    
    EXPECT_GT(commandManager_->getUndoStackSize(), 0);
    EXPECT_LT(commandManager_->getUndoStackSize(), 100);
}

TEST_F(VersionControlTest, CommandManager_MaxUndoSteps) {
    commandManager_->setMaxUndoSteps(5);
    
    for (int i = 0; i < 10; ++i) {
        commandManager_->executeCommand(
            std::make_unique<TestCommand>(&testValue_, i, "Command " + std::to_string(i))
        );
    }
    
    EXPECT_EQ(commandManager_->getUndoStackSize(), 5);
}

TEST_F(VersionControlTest, ChangeTracker_RecordChange) {
    ChangeTracker::ChangeRecord record;
    record.objectId = "object1";
    record.propertyName = "position";
    record.oldValue = "0,0,0";
    record.newValue = "1,2,3";
    record.userId = "user1";
    record.sessionId = "session1";
    
    changeTracker_->recordChange(record);
    
    auto history = changeTracker_->getChangeHistory("object1");
    
    EXPECT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].objectId, "object1");
    EXPECT_EQ(history[0].propertyName, "position");
    EXPECT_EQ(history[0].oldValue, "0,0,0");
    EXPECT_EQ(history[0].newValue, "1,2,3");
}

TEST_F(VersionControlTest, ChangeTracker_MultipleChanges) {
    for (int i = 0; i < 10; ++i) {
        ChangeTracker::ChangeRecord record;
        record.objectId = "object1";
        record.propertyName = "value";
        record.oldValue = std::to_string(i);
        record.newValue = std::to_string(i + 1);
        record.userId = "user1";
        record.sessionId = "session1";
        
        changeTracker_->recordChange(record);
    }
    
    auto history = changeTracker_->getChangeHistory("object1");
    
    EXPECT_EQ(history.size(), 10);
}

TEST_F(VersionControlTest, ChangeTracker_Clear) {
    ChangeTracker::ChangeRecord record;
    record.objectId = "object1";
    record.propertyName = "value";
    record.oldValue = "0";
    record.newValue = "1";
    
    changeTracker_->recordChange(record);
    
    EXPECT_FALSE(changeTracker_->getChangeHistory().empty());
    
    changeTracker_->clear();
    
    EXPECT_TRUE(changeTracker_->getChangeHistory().empty());
}

TEST_F(VersionControlTest, VersionManager_CreateVersion) {
    std::string versionId = versionManager_->createVersion("Initial version");
    
    EXPECT_FALSE(versionId.empty());
    
    auto versionInfo = versionManager_->getVersionInfo(versionId);
    
    EXPECT_EQ(versionInfo.versionId, versionId);
    EXPECT_EQ(versionInfo.description, "Initial version");
    EXPECT_FALSE(versionInfo.parentId.empty());
}

TEST_F(VersionControlTest, VersionManager_VersionHistory) {
    versionManager_->createVersion("Version 1");
    versionManager_->createVersion("Version 2");
    versionManager_->createVersion("Version 3");
    
    auto history = versionManager_->getVersionHistory();
    
    EXPECT_EQ(history.size(), 3);
    EXPECT_EQ(history[0].description, "Version 1");
    EXPECT_EQ(history[1].description, "Version 2");
    EXPECT_EQ(history[2].description, "Version 3");
}

TEST_F(VersionControlTest, VersionManager_CurrentVersion) {
    std::string versionId1 = versionManager_->createVersion("Version 1");
    std::string versionId2 = versionManager_->createVersion("Version 2");
    
    std::string currentVersion = versionManager_->getCurrentVersion();
    
    EXPECT_EQ(currentVersion, versionId2);
}

TEST_F(VersionControlTest, VersionManager_MaxVersions) {
    versionManager_->setMaxVersions(5);
    
    for (int i = 0; i < 10; ++i) {
        versionManager_->createVersion("Version " + std::to_string(i));
    }
    
    auto history = versionManager_->getVersionHistory();
    
    EXPECT_LE(history.size(), 5);
}

TEST_F(VersionControlTest, VersionManager_DeleteVersion) {
    std::string versionId = versionManager_->createVersion("Test version");
    
    EXPECT_FALSE(versionManager_->getVersionInfo(versionId).versionId.empty());
    
    bool deleted = versionManager_->deleteVersion(versionId);
    
    EXPECT_TRUE(deleted);
    
    auto versionInfo = versionManager_->getVersionInfo(versionId);
    
    EXPECT_TRUE(versionInfo.versionId.empty());
}

TEST_F(VersionControlTest, Integration_CommandWithVersion) {
    std::string versionId1 = versionManager_->createVersion("Initial state");
    
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 10, "Set to 10"));
    commandManager_->executeCommand(std::make_unique<TestCommand>(&testValue_, 20, "Set to 20"));
    
    std::string versionId2 = versionManager_->createVersion("Modified state");
    
    EXPECT_EQ(testValue_, 20);
    
    commandManager_->undo();
    commandManager_->undo();
    
    EXPECT_EQ(testValue_, 0);
}

TEST_F(VersionControlTest, StressTest_ManyCommands) {
    const int numCommands = 1000;
    
    TestUtils::measureTime("Execute 1000 commands", [this, numCommands]() {
        for (int i = 0; i < numCommands; ++i) {
            commandManager_->executeCommand(
                std::make_unique<TestCommand>(&testValue_, i, "Command " + std::to_string(i))
            );
        }
    });
    
    EXPECT_EQ(testValue_, numCommands - 1);
}

TEST_F(VersionControlTest, StressTest_ManyChanges) {
    const int numChanges = 1000;
    
    TestUtils::measureTime("Record 1000 changes", [this, numChanges]() {
        for (int i = 0; i < numChanges; ++i) {
            ChangeTracker::ChangeRecord record;
            record.objectId = "object1";
            record.propertyName = "value";
            record.oldValue = std::to_string(i);
            record.newValue = std::to_string(i + 1);
            record.userId = "user1";
            record.sessionId = "session1";
            
            changeTracker_->recordChange(record);
        }
    });
    
    auto history = changeTracker_->getChangeHistory("object1");
    EXPECT_EQ(history.size(), numChanges);
}
