#include <gtest/gtest.h>
#include "../../src/animation/skeleton.h"
#include "../utils/test_utils.h"
#include <glm/glm.hpp>

using namespace v3d::animation;
using namespace v3d::test;

class SkeletonTest : public ::testing::Test {
protected:
    void SetUp() override {
        skeleton_ = std::make_shared<Skeleton>();
    }

    void TearDown() override {
    }

    std::shared_ptr<Skeleton> skeleton_;
};

TEST_F(SkeletonTest, CreateBone) {
    Bone* bone = skeleton_->createBone("Root");
    
    ASSERT_NE(bone, nullptr);
    EXPECT_EQ(bone->getName(), "Root");
    EXPECT_EQ(skeleton_->getBoneCount(), 1);
}

TEST_F(SkeletonTest, CreateBoneHierarchy) {
    Bone* root = skeleton_->createBone("Root");
    Bone* child1 = skeleton_->createBone("Child1", root);
    Bone* child2 = skeleton_->createBone("Child2", root);
    Bone* grandchild = skeleton_->createBone("Grandchild", child1);
    
    EXPECT_EQ(skeleton_->getBoneCount(), 4);
    
    EXPECT_EQ(child1->getParent(), root);
    EXPECT_EQ(child2->getParent(), root);
    EXPECT_EQ(grandchild->getParent(), child1);
    
    EXPECT_EQ(root->getChildren().size(), 2);
    EXPECT_EQ(child1->getChildren().size(), 1);
    EXPECT_EQ(child2->getChildren().size(), 0);
}

TEST_F(SkeletonTest, GetBoneByName) {
    skeleton_->createBone("Root");
    skeleton_->createBone("Child1");
    skeleton_->createBone("Child2");
    
    Bone* bone = skeleton_->getBone("Child1");
    
    ASSERT_NE(bone, nullptr);
    EXPECT_EQ(bone->getName(), "Child1");
}

TEST_F(SkeletonTest, GetBoneByIndex) {
    Bone* bone1 = skeleton_->createBone("Bone1");
    Bone* bone2 = skeleton_->createBone("Bone2");
    Bone* bone3 = skeleton_->createBone("Bone3");
    
    EXPECT_EQ(skeleton_->getBone(0), bone1);
    EXPECT_EQ(skeleton_->getBone(1), bone2);
    EXPECT_EQ(skeleton_->getBone(2), bone3);
}

TEST_F(SkeletonTest, SetRootBone) {
    Bone* root = skeleton_->createBone("Root");
    skeleton_->setRootBone(root);
    
    EXPECT_EQ(skeleton_->getRootBone(), root);
}

TEST_F(SkeletonTest, BoneTransform) {
    Bone* bone = skeleton_->createBone("TestBone");
    
    Transform transform;
    transform.position = glm::vec3(1.0f, 2.0f, 3.0f);
    transform.rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);
    
    bone->setLocalTransform(transform);
    
    Transform result = bone->getLocalTransform();
    
    EXPECT_EQ(result.position, transform.position);
    EXPECT_EQ(result.rotation, transform.rotation);
    EXPECT_EQ(result.scale, transform.scale);
}

TEST_F(SkeletonTest, UpdateWorldTransform) {
    Bone* root = skeleton_->createBone("Root");
    Bone* child = skeleton_->createBone("Child", root);
    
    Transform rootTransform;
    rootTransform.position = glm::vec3(0.0f, 1.0f, 0.0f);
    root->setLocalTransform(rootTransform);
    
    Transform childTransform;
    childTransform.position = glm::vec3(0.0f, 1.0f, 0.0f);
    child->setLocalTransform(childTransform);
    
    skeleton_->update();
    
    Transform childWorld = child->getWorldTransform();
    
    EXPECT_FLOAT_EQ(childWorld.position.y, 2.0f);
}

TEST_F(SkeletonTest, CalculateBindPose) {
    Bone* root = skeleton_->createBone("Root");
    Bone* child = skeleton_->createBone("Child", root);
    
    skeleton_->calculateBindPose();
    
    EXPECT_NE(skeleton_->getInverseBindPoseMatrices().size(), 0);
}

TEST_F(SkeletonTest, GetBoneMatrices) {
    skeleton_->createBone("Root");
    skeleton_->createBone("Child1");
    skeleton_->createBone("Child2");
    
    skeleton_->update();
    
    auto matrices = skeleton_->getBoneMatrices();
    
    EXPECT_EQ(matrices.size(), 3);
}

TEST_F(SkeletonTest, ResetToBindPose) {
    Bone* root = skeleton_->createBone("Root");
    
    Transform transform;
    transform.position = glm::vec3(10.0f, 0.0f, 0.0f);
    root->setLocalTransform(transform);
    
    skeleton_->calculateBindPose();
    
    EXPECT_FLOAT_EQ(root->getLocalTransform().position.x, 10.0f);
}

TEST_F(SkeletonTest, BoneAnimatedFlag) {
    Bone* bone = skeleton_->createBone("TestBone");
    
    EXPECT_FALSE(bone->isAnimated());
    
    bone->setAnimated(true);
    
    EXPECT_TRUE(bone->isAnimated());
}

TEST_F(SkeletonTest, LargeSkeleton) {
    const int boneCount = 100;
    
    TestUtils::measureTime("Create large skeleton", [this, boneCount]() {
        Bone* prevBone = nullptr;
        for (int i = 0; i < boneCount; ++i) {
            std::string boneName = "Bone" + std::to_string(i);
            prevBone = skeleton_->createBone(boneName, prevBone);
            if (i == 0) {
                skeleton_->setRootBone(prevBone);
            }
        }
    });
    
    EXPECT_EQ(skeleton_->getBoneCount(), boneCount);
}

class PoseTest : public ::testing::Test {
protected:
    void SetUp() override {
        skeleton_ = std::make_shared<Skeleton>();
        skeleton_->createBone("Root");
        skeleton_->createBone("Child1");
        skeleton_->createBone("Child2");
        skeleton_->calculateBindPose();
        
        pose_ = std::make_shared<Pose>(skeleton_.get());
    }

    void TearDown() override {
    }

    std::shared_ptr<Skeleton> skeleton_;
    std::shared_ptr<Pose> pose_;
};

TEST_F(PoseTest, SetGetLocalTransform) {
    Transform transform;
    transform.position = glm::vec3(1.0f, 2.0f, 3.0f);
    
    pose_->setLocalTransform(0, transform);
    
    Transform result = pose_->getLocalTransform(0);
    
    EXPECT_EQ(result.position, transform.position);
}

TEST_F(PoseTest, SetGetWorldTransform) {
    Transform transform;
    transform.position = glm::vec3(1.0f, 0.0f, 0.0f);
    
    pose_->setWorldTransform(0, transform);
    
    Transform result = pose_->getWorldTransform(0);
    
    EXPECT_EQ(result.position, transform.position);
}

TEST_F(PoseTest, CalculateWorldTransforms) {
    Transform transform0;
    transform0.position = glm::vec3(0.0f, 1.0f, 0.0f);
    pose_->setLocalTransform(0, transform0);
    
    Transform transform1;
    transform1.position = glm::vec3(0.0f, 1.0f, 0.0f);
    pose_->setLocalTransform(1, transform1);
    
    pose_->calculateWorldTransforms();
    
    Transform world1 = pose_->getWorldTransform(1);
    
    EXPECT_FLOAT_EQ(world1.position.y, 2.0f);
}

TEST_F(PoseTest, BlendPoses) {
    Transform transform1;
    transform1.position = glm::vec3(0.0f, 0.0f, 0.0f);
    pose_->setLocalTransform(0, transform1);
    
    Pose pose2(skeleton_.get());
    Transform transform2;
    transform2.position = glm::vec3(10.0f, 0.0f, 0.0f);
    pose2.setLocalTransform(0, transform2);
    
    pose_->blend(pose2, 0.5f);
    
    Transform result = pose_->getLocalTransform(0);
    
    EXPECT_FLOAT_EQ(result.position.x, 5.0f);
}

TEST_F(PoseTest, AddPoses) {
    Transform transform1;
    transform1.position = glm::vec3(1.0f, 0.0f, 0.0f);
    pose_->setLocalTransform(0, transform1);
    
    Pose pose2(skeleton_.get());
    Transform transform2;
    transform2.position = glm::vec3(2.0f, 0.0f, 0.0f);
    pose2.setLocalTransform(0, transform2);
    
    pose_->add(pose2, 0.5f);
    
    Transform result = pose_->getLocalTransform(0);
    
    EXPECT_FLOAT_EQ(result.position.x, 2.0f);
}

TEST_F(PoseTest, IsValid) {
    EXPECT_TRUE(pose_->isValid());
    
    Pose invalidPose;
    EXPECT_FALSE(invalidPose.isValid());
}

TEST_F(PoseTest, GetBoneCount) {
    EXPECT_EQ(pose_->getBoneCount(), 3);
}
