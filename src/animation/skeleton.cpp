#include "skeleton.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>

namespace v3d {
namespace animation {

glm::mat4 Transform::toMatrix() const {
    glm::mat4 matrix = glm::mat4(1.0f);
    matrix = glm::translate(matrix, position);
    matrix = matrix * glm::mat4_cast(rotation);
    matrix = glm::scale(matrix, scale);
    return matrix;
}

Transform Transform::fromMatrix(const glm::mat4& matrix) {
    Transform transform;

    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(matrix, transform.scale, transform.rotation, transform.position, skew, perspective);

    return transform;
}

Transform Transform::interpolate(const Transform& a, const Transform& b, float t) {
    Transform result;
    result.position = glm::mix(a.position, b.position, t);
    result.rotation = glm::slerp(a.rotation, b.rotation, t);
    result.scale = glm::mix(a.scale, b.scale, t);
    return result;
}

Transform Transform::inverse() const {
    Transform result;
    result.rotation = glm::inverse(rotation);
    result.scale = glm::vec3(1.0f) / scale;
    result.position = -(result.rotation * (result.scale * position));
    return result;
}

Transform Transform::operator*(const Transform& other) const {
    Transform result;
    result.rotation = rotation * other.rotation;
    result.scale = scale * other.scale;
    result.position = position + (rotation * (scale * other.position));
    return result;
}

Bone::Bone()
    : index_(-1)
    , parent_(nullptr)
    , animated_(false) {
}

Bone::Bone(const std::string& name)
    : name_(name)
    , index_(-1)
    , parent_(nullptr)
    , animated_(false) {
}

std::string Bone::getName() const {
    return name_;
}

void Bone::setName(const std::string& name) {
    name_ = name;
}

int Bone::getIndex() const {
    return index_;
}

void Bone::setIndex(int index) {
    index_ = index;
}

Transform Bone::getLocalTransform() const {
    return localTransform_;
}

void Bone::setLocalTransform(const Transform& transform) {
    localTransform_ = transform;
}

Transform Bone::getWorldTransform() const {
    return worldTransform_;
}

void Bone::setWorldTransform(const Transform& transform) {
    worldTransform_ = transform;
}

Transform Bone::getOffsetMatrix() const {
    return offsetMatrix_;
}

void Bone::setOffsetMatrix(const Transform& transform) {
    offsetMatrix_ = transform;
}

Bone* Bone::getParent() const {
    return parent_;
}

void Bone::setParent(Bone* parent) {
    parent_ = parent;
}

const std::vector<Bone*>& Bone::getChildren() const {
    return children_;
}

void Bone::addChild(Bone* child) {
    if (child && std::find(children_.begin(), children_.end(), child) == children_.end()) {
        children_.push_back(child);
        child->setParent(this);
    }
}

void Bone::removeChild(Bone* child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        (*it)->setParent(nullptr);
        children_.erase(it);
    }
}

void Bone::updateWorldTransform(const Transform& parentWorldTransform) {
    if (parent_) {
        worldTransform_ = parentWorldTransform * localTransform_;
    } else {
        worldTransform_ = localTransform_;
    }

    for (auto* child : children_) {
        child->updateWorldTransform(worldTransform_);
    }
}

bool Bone::isAnimated() const {
    return animated_;
}

void Bone::setAnimated(bool animated) {
    animated_ = animated;
}

Skeleton::Skeleton()
    : rootBone_(nullptr) {
}

Skeleton::~Skeleton() {
}

Bone* Skeleton::createBone(const std::string& name, Bone* parent) {
    auto bone = std::make_unique<Bone>(name);
    bone->setIndex(static_cast<int>(bones_.size()));

    Bone* rawBone = bone.get();
    bones_.push_back(std::move(bone));
    boneMap_[name] = rawBone;

    if (parent) {
        parent->addChild(rawBone);
    } else if (!rootBone_) {
        rootBone_ = rawBone;
    }

    return rawBone;
}

Bone* Skeleton::getBone(const std::string& name) const {
    auto it = boneMap_.find(name);
    return it != boneMap_.end() ? it->second : nullptr;
}

Bone* Skeleton::getBone(int index) const {
    if (index >= 0 && index < static_cast<int>(bones_.size())) {
        return bones_[index].get();
    }
    return nullptr;
}

std::vector<Bone*> Skeleton::getBones() const {
    std::vector<Bone*> result;
    result.reserve(bones_.size());
    for (const auto& bone : bones_) {
        result.push_back(bone.get());
    }
    return result;
}

size_t Skeleton::getBoneCount() const {
    return bones_.size();
}

void Skeleton::setRootBone(Bone* bone) {
    rootBone_ = bone;
}

Bone* Skeleton::getRootBone() const {
    return rootBone_;
}

void Skeleton::update() {
    if (rootBone_) {
        rootBone_->updateWorldTransform(Transform());
    }
}

void Skeleton::resetToBindPose() {
    for (auto& bone : bones_) {
        bone->setLocalTransform(bone->getOffsetMatrix());
    }
    update();
}

std::string Skeleton::getName() const {
    return name_;
}

void Skeleton::setName(const std::string& name) {
    name_ = name;
}

void Skeleton::calculateBindPose() {
    if (rootBone_) {
        rootBone_->updateWorldTransform(Transform());
    }

    for (auto& bone : bones_) {
        bone->setOffsetMatrix(bone->getWorldTransform());
    }
}

std::vector<glm::mat4> Skeleton::getBoneMatrices() const {
    std::vector<glm::mat4> matrices;
    matrices.reserve(bones_.size());

    for (const auto& bone : bones_) {
        matrices.push_back(bone->getWorldTransform().toMatrix());
    }

    return matrices;
}

std::vector<glm::mat4> Skeleton::getInverseBindPoseMatrices() const {
    std::vector<glm::mat4> matrices;
    matrices.reserve(bones_.size());

    for (const auto& bone : bones_) {
        matrices.push_back(glm::inverse(bone->getOffsetMatrix().toMatrix()));
    }

    return matrices;
}

Pose::Pose()
    : skeleton_(nullptr) {
}

Pose::Pose(const Skeleton* skeleton)
    : skeleton_(skeleton) {
    if (skeleton_) {
        localTransforms_.resize(skeleton_->getBoneCount());
        worldTransforms_.resize(skeleton_->getBoneCount());

        for (size_t i = 0; i < skeleton_->getBoneCount(); ++i) {
            if (auto* bone = skeleton_->getBone(static_cast<int>(i))) {
                localTransforms_[i] = bone->getLocalTransform();
            }
        }
    }
}

void Pose::setSkeleton(const Skeleton* skeleton) {
    skeleton_ = skeleton;
    if (skeleton_) {
        localTransforms_.resize(skeleton_->getBoneCount());
        worldTransforms_.resize(skeleton_->getBoneCount());
    }
}

const Skeleton* Pose::getSkeleton() const {
    return skeleton_;
}

size_t Pose::getBoneCount() const {
    return localTransforms_.size();
}

Transform Pose::getLocalTransform(int boneIndex) const {
    if (boneIndex >= 0 && boneIndex < static_cast<int>(localTransforms_.size())) {
        return localTransforms_[boneIndex];
    }
    return Transform();
}

void Pose::setLocalTransform(int boneIndex, const Transform& transform) {
    if (boneIndex >= 0 && boneIndex < static_cast<int>(localTransforms_.size())) {
        localTransforms_[boneIndex] = transform;
    }
}

Transform Pose::getWorldTransform(int boneIndex) const {
    if (boneIndex >= 0 && boneIndex < static_cast<int>(worldTransforms_.size())) {
        return worldTransforms_[boneIndex];
    }
    return Transform();
}

void Pose::setWorldTransform(int boneIndex, const Transform& transform) {
    if (boneIndex >= 0 && boneIndex < static_cast<int>(worldTransforms_.size())) {
        worldTransforms_[boneIndex] = transform;
    }
}

void Pose::blend(const Pose& other, float weight) {
    if (!skeleton_ || !other.skeleton_ || skeleton_ != other.skeleton_) {
        return;
    }

    for (size_t i = 0; i < localTransforms_.size(); ++i) {
        localTransforms_[i] = Transform::interpolate(
            localTransforms_[i],
            other.localTransforms_[i],
            weight
        );
    }
}

void Pose::add(const Pose& other, float weight) {
    if (!skeleton_ || !other.skeleton_ || skeleton_ != other.skeleton_) {
        return;
    }

    for (size_t i = 0; i < localTransforms_.size(); ++i) {
        localTransforms_[i].position += other.localTransforms_[i].position * weight;
        localTransforms_[i].rotation = glm::slerp(
            localTransforms_[i].rotation,
            localTransforms_[i].rotation * other.localTransforms_[i].rotation,
            weight
        );
    }
}

void Pose::multiply(const Pose& other) {
    if (!skeleton_ || !other.skeleton_ || skeleton_ != other.skeleton_) {
        return;
    }

    for (size_t i = 0; i < localTransforms_.size(); ++i) {
        localTransforms_[i] = localTransforms_[i] * other.localTransforms_[i];
    }
}

void Pose::calculateWorldTransforms() {
    if (!skeleton_) {
        return;
    }

    for (size_t i = 0; i < skeleton_->getBoneCount(); ++i) {
        auto* bone = skeleton_->getBone(static_cast<int>(i));
        if (!bone) continue;

        if (bone->getParent()) {
            int parentIndex = bone->getParent()->getIndex();
            worldTransforms_[i] = worldTransforms_[parentIndex] * localTransforms_[i];
        } else {
            worldTransforms_[i] = localTransforms_[i];
        }
    }
}

void Pose::calculateLocalTransforms() {
    if (!skeleton_) {
        return;
    }

    for (size_t i = 0; i < skeleton_->getBoneCount(); ++i) {
        auto* bone = skeleton_->getBone(static_cast<int>(i));
        if (!bone) continue;

        if (bone->getParent()) {
            int parentIndex = bone->getParent()->getIndex();
            auto parentWorldInverse = worldTransforms_[parentIndex].inverse();
            localTransforms_[i] = parentWorldInverse * worldTransforms_[i];
        } else {
            localTransforms_[i] = worldTransforms_[i];
        }
    }
}

bool Pose::isValid() const {
    return skeleton_ != nullptr &&
           localTransforms_.size() == skeleton_->getBoneCount() &&
           worldTransforms_.size() == skeleton_->getBoneCount();
}

}
}