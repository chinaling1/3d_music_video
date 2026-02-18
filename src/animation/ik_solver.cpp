#include "ik_solver.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>
#include <algorithm>

namespace v3d {
namespace animation {

IKSolver::IKSolver()
    : maxIterations_(10)
    , tolerance_(0.001f)
    , chainLength_(2) {
}

IKSolver::~IKSolver() {
}

void IKSolver::setMaxIterations(int iterations) {
    maxIterations_ = std::max(1, iterations);
}

int IKSolver::getMaxIterations() const {
    return maxIterations_;
}

void IKSolver::setTolerance(float tolerance) {
    tolerance_ = std::max(0.0001f, tolerance);
}

float IKSolver::getTolerance() const {
    return tolerance_;
}

void IKSolver::setChainLength(int length) {
    chainLength_ = std::max(2, length);
}

int IKSolver::getChainLength() const {
    return chainLength_;
}

std::vector<Bone*> IKSolver::buildIKChain(Bone* tipBone, int length) {
    std::vector<Bone*> chain;
    Bone* current = tipBone;

    for (int i = 0; i < length && current != nullptr; ++i) {
        chain.push_back(current);
        current = current->getParent();
    }

    std::reverse(chain.begin(), chain.end());
    return chain;
}

TwoBoneIKSolver::TwoBoneIKSolver()
    : poleVector_(0.0f, 1.0f, 0.0f)
    , poleEnabled_(false) {
}

bool TwoBoneIKSolver::solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) {
    Bone* tipBone = skeleton->getBone(tipBoneName);
    if (!tipBone) {
        return false;
    }

    auto chain = buildIKChain(tipBone, chainLength_);
    if (chain.size() < 2) {
        return false;
    }

    Bone* rootBone = chain[0];
    Bone* midBone = chain[1];
    Bone* endBone = chain.size() > 2 ? chain[2] : tipBone;

    glm::vec3 rootPos = rootBone->getWorldTransform().position;
    glm::vec3 midPos = midBone->getWorldTransform().position;
    glm::vec3 endPos = endBone->getWorldTransform().position;
    
    glm::vec3 adjustedTarget = targetPosition;

    float upperArmLength = glm::length(midPos - rootPos);
    float lowerArmLength = glm::length(endPos - midPos);
    float totalLength = upperArmLength + lowerArmLength;

    glm::vec3 targetDir = adjustedTarget - rootPos;
    float targetDist = glm::length(targetDir);

    if (targetDist > totalLength * 0.9999f) {
        targetDir = glm::normalize(targetDir);
        adjustedTarget = rootPos + targetDir * totalLength;
        targetDist = totalLength;
    }

    float cosAngle = (upperArmLength * upperArmLength + targetDist * targetDist - lowerArmLength * lowerArmLength) /
                     (2.0f * upperArmLength * targetDist);
    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);
    float rootAngle = std::acos(cosAngle);

    cosAngle = (upperArmLength * upperArmLength + lowerArmLength * lowerArmLength - targetDist * targetDist) /
               (2.0f * upperArmLength * lowerArmLength);
    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);
    float midAngle = std::acos(cosAngle);

    glm::vec3 toTarget = glm::normalize(targetPosition - rootPos);
    glm::quat rootRotation = glm::rotation(glm::vec3(1.0f, 0.0f, 0.0f), toTarget);

    if (poleEnabled_) {
        glm::vec3 poleDir = glm::normalize(poleVector_);
        glm::vec3 chainDir = glm::normalize(endPos - rootPos);
        glm::vec3 right = glm::cross(chainDir, poleDir);
        if (glm::length(right) < 0.001f) {
            right = glm::vec3(0.0f, 0.0f, 1.0f);
        } else {
            right = glm::normalize(right);
        }
        glm::vec3 up = glm::cross(right, chainDir);

        glm::mat3 rotationMatrix;
        rotationMatrix[0] = toTarget;
        rotationMatrix[1] = up;
        rotationMatrix[2] = right;

        rootRotation = glm::quat_cast(rotationMatrix);
    }

    Transform rootTransform = rootBone->getLocalTransform();
    rootTransform.rotation = rootRotation;
    rootBone->setLocalTransform(rootTransform);

    skeleton->update();

    midPos = midBone->getWorldTransform().position;

    glm::vec3 toEnd = glm::normalize(endPos - midPos);
    glm::vec3 toRoot = glm::normalize(rootPos - midPos);

    glm::quat midRotation = glm::rotation(toEnd, toRoot);
    midRotation = glm::angleAxis(midAngle, glm::vec3(0.0f, 0.0f, 1.0f)) * midRotation;

    Transform midTransform = midBone->getLocalTransform();
    midTransform.rotation = midRotation;
    midBone->setLocalTransform(midTransform);

    skeleton->update();

    return true;
}

void TwoBoneIKSolver::setPoleVector(const glm::vec3& pole) {
    poleVector_ = glm::normalize(pole);
}

glm::vec3 TwoBoneIKSolver::getPoleVector() const {
    return poleVector_;
}

void TwoBoneIKSolver::setPoleEnabled(bool enabled) {
    poleEnabled_ = enabled;
}

bool TwoBoneIKSolver::isPoleEnabled() const {
    return poleEnabled_;
}

FABRIKSolver::FABRIKSolver()
    : targetRotation_(1.0f, 0.0f, 0.0f, 0.0f)
    , targetRotationEnabled_(false) {
}

bool FABRIKSolver::solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) {
    Bone* tipBone = skeleton->getBone(tipBoneName);
    if (!tipBone) {
        return false;
    }

    auto chain = buildIKChain(tipBone, chainLength_);
    if (chain.size() < 2) {
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<float> lengths;

    for (size_t i = 0; i < chain.size(); ++i) {
        positions.push_back(chain[i]->getWorldTransform().position);
    }

    for (size_t i = 0; i < chain.size() - 1; ++i) {
        float length = glm::length(positions[i + 1] - positions[i]);
        lengths.push_back(length);
    }

    float totalLength = 0.0f;
    for (float length : lengths) {
        totalLength += length;
    }

    glm::vec3 rootPos = positions[0];
    glm::vec3 targetPos = targetPosition;

    float targetDist = glm::length(targetPos - rootPos);
    if (targetDist > totalLength) {
        for (size_t i = 0; i < chain.size() - 1; ++i) {
            glm::vec3 dir = glm::normalize(targetPos - positions[i]);
            positions[i + 1] = positions[i] + dir * lengths[i];
        }
    } else {
        for (int iter = 0; iter < maxIterations_; ++iter) {
            positions.back() = targetPos;

            for (int i = static_cast<int>(chain.size()) - 2; i >= 0; --i) {
                glm::vec3 dir = glm::normalize(positions[i] - positions[i + 1]);
                positions[i] = positions[i + 1] + dir * lengths[i];
            }

            positions[0] = rootPos;

            for (size_t i = 0; i < chain.size() - 1; ++i) {
                glm::vec3 dir = glm::normalize(positions[i + 1] - positions[i]);
                positions[i + 1] = positions[i] + dir * lengths[i];
            }
        }
    }

    for (size_t i = 0; i < chain.size(); ++i) {
        Transform transform = chain[i]->getLocalTransform();
        transform.position = positions[i];
        chain[i]->setLocalTransform(transform);
    }

    if (targetRotationEnabled_) {
        if (Bone* tip = chain.back()) {
            Transform transform = tip->getLocalTransform();
            transform.rotation = targetRotation_;
            tip->setLocalTransform(transform);
        }
    }

    skeleton->update();

    return true;
}

void FABRIKSolver::setTargetRotation(const glm::quat& rotation) {
    targetRotation_ = rotation;
}

glm::quat FABRIKSolver::getTargetRotation() const {
    return targetRotation_;
}

void FABRIKSolver::setTargetRotationEnabled(bool enabled) {
    targetRotationEnabled_ = enabled;
}

bool FABRIKSolver::isTargetRotationEnabled() const {
    return targetRotationEnabled_;
}

CCDSolver::CCDSolver() {
}

bool CCDSolver::solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) {
    Bone* tipBone = skeleton->getBone(tipBoneName);
    if (!tipBone) {
        return false;
    }

    auto chain = buildIKChain(tipBone, chainLength_);
    if (chain.size() < 2) {
        return false;
    }

    for (int iter = 0; iter < maxIterations_; ++iter) {
        for (int i = static_cast<int>(chain.size()) - 1; i >= 0; --i) {
            Bone* bone = chain[i];
            Bone* tip = chain.back();

            glm::vec3 tipPos = tip->getWorldTransform().position;
            glm::vec3 bonePos = bone->getWorldTransform().position;

            glm::vec3 toTip = glm::normalize(tipPos - bonePos);
            glm::vec3 toTarget = glm::normalize(targetPosition - bonePos);

            float dot = glm::dot(toTip, toTarget);
            if (dot > 0.9999f) {
                continue;
            }

            glm::vec3 axis = glm::cross(toTip, toTarget);
            float angle = std::acos(std::clamp(dot, -1.0f, 1.0f));

            if (glm::length(axis) < 0.0001f) {
                continue;
            }

            axis = glm::normalize(axis);
            glm::quat rotation = glm::angleAxis(angle, axis);

            Transform transform = bone->getLocalTransform();
            transform.rotation = rotation * transform.rotation;
            bone->setLocalTransform(transform);

            skeleton->update();

            tipPos = tip->getWorldTransform().position;
            if (glm::length(tipPos - targetPosition) < tolerance_) {
                return true;
            }
        }
    }

    return true;
}

JacobianSolver::JacobianSolver()
    : damping_(0.1f) {
}

bool JacobianSolver::solve(const std::string& tipBoneName, const glm::vec3& targetPosition, Skeleton* skeleton) {
    Bone* tipBone = skeleton->getBone(tipBoneName);
    if (!tipBone) {
        return false;
    }

    auto chain = buildIKChain(tipBone, chainLength_);
    if (chain.size() < 2) {
        return false;
    }

    for (int iter = 0; iter < maxIterations_; ++iter) {
        Bone* tip = chain.back();
        glm::vec3 tipPos = tip->getWorldTransform().position;
        glm::vec3 error = targetPosition - tipPos;

        if (glm::length(error) < tolerance_) {
            return true;
        }

        int numJoints = static_cast<int>(chain.size());
        
        std::vector<glm::vec3> jacobianColumns(numJoints);
        for (int i = 0; i < numJoints; ++i) {
            Bone* bone = chain[i];
            glm::vec3 bonePos = bone->getWorldTransform().position;
            glm::vec3 jointToTip = tipPos - bonePos;
            glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
            jacobianColumns[i] = glm::cross(axis, jointToTip);
        }

        std::vector<float> deltaTheta(numJoints, 0.0f);
        
        float dampingSq = damping_ * damping_;
        for (int i = 0; i < numJoints; ++i) {
            float jT_error = glm::dot(jacobianColumns[i], error);
            float jT_j = glm::dot(jacobianColumns[i], jacobianColumns[i]);
            deltaTheta[i] = jT_error / (jT_j + dampingSq);
        }

        for (int i = 0; i < numJoints; ++i) {
            Transform transform = chain[i]->getLocalTransform();
            glm::quat deltaRot = glm::angleAxis(deltaTheta[i], glm::vec3(0.0f, 1.0f, 0.0f));
            transform.rotation = deltaRot * transform.rotation;
            chain[i]->setLocalTransform(transform);
        }

        skeleton->update();

        if (secondaryTask_) {
            secondaryTask_(skeleton);
        }
    }

    return true;
}

void JacobianSolver::setDamping(float damping) {
    damping_ = std::max(0.0f, damping);
}

float JacobianSolver::getDamping() const {
    return damping_;
}

void JacobianSolver::setSecondaryTask(std::function<void(Skeleton*)> task) {
    secondaryTask_ = task;
}

IKController::IKController() {
}

IKController::~IKController() {
}

void IKController::addIKSolver(const std::string& name, std::unique_ptr<IKSolver> solver) {
    IKHandle handle;
    handle.solver = std::move(solver);
    handle.enabled = true;
    ikHandles_[name] = std::move(handle);
}

void IKController::removeIKSolver(const std::string& name) {
    ikHandles_.erase(name);
}

IKSolver* IKController::getIKSolver(const std::string& name) const {
    auto it = ikHandles_.find(name);
    return it != ikHandles_.end() ? it->second.solver.get() : nullptr;
}

void IKController::solveAll(Skeleton* skeleton) {
    for (auto& pair : ikHandles_) {
        if (pair.second.enabled && pair.second.solver) {
            pair.second.solver->solve(pair.second.tipBoneName, pair.second.targetPosition, skeleton);
        }
    }
}

void IKController::setIKTarget(const std::string& ikName, const glm::vec3& position) {
    auto it = ikHandles_.find(ikName);
    if (it != ikHandles_.end()) {
        it->second.targetPosition = position;
    }
}

glm::vec3 IKController::getIKTarget(const std::string& ikName) const {
    auto it = ikHandles_.find(ikName);
    return it != ikHandles_.end() ? it->second.targetPosition : glm::vec3(0.0f);
}

void IKController::setIKEnabled(const std::string& ikName, bool enabled) {
    auto it = ikHandles_.find(ikName);
    if (it != ikHandles_.end()) {
        it->second.enabled = enabled;
    }
}

bool IKController::isIKEnabled(const std::string& ikName) const {
    auto it = ikHandles_.find(ikName);
    return it != ikHandles_.end() ? it->second.enabled : false;
}

void IKController::clear() {
    ikHandles_.clear();
}

LookAtIK::LookAtIK()
    : upVector_(0.0f, 1.0f, 0.0f)
    , weight_(1.0f) {
}

void LookAtIK::setTargetBone(const std::string& boneName) {
    targetBone_ = boneName;
}

std::string LookAtIK::getTargetBone() const {
    return targetBone_;
}

void LookAtIK::setLookAtTarget(const glm::vec3& target) {
    lookAtTarget_ = target;
}

glm::vec3 LookAtIK::getLookAtTarget() const {
    return lookAtTarget_;
}

void LookAtIK::setUpVector(const glm::vec3& up) {
    upVector_ = glm::normalize(up);
}

glm::vec3 LookAtIK::getUpVector() const {
    return upVector_;
}

void LookAtIK::setWeight(float weight) {
    weight_ = std::clamp(weight, 0.0f, 1.0f);
}

float LookAtIK::getWeight() const {
    return weight_;
}

void LookAtIK::apply(Skeleton* skeleton) {
    if (targetBone_.empty() || weight_ <= 0.0f) {
        return;
    }

    Bone* bone = skeleton->getBone(targetBone_);
    if (!bone) {
        return;
    }

    glm::vec3 bonePos = bone->getWorldTransform().position;
    glm::vec3 toTarget = glm::normalize(lookAtTarget_ - bonePos);

    glm::vec3 right = glm::cross(toTarget, upVector_);
    if (glm::length(right) < 0.001f) {
        right = glm::vec3(0.0f, 0.0f, 1.0f);
    } else {
        right = glm::normalize(right);
    }

    glm::vec3 up = glm::cross(right, toTarget);

    glm::mat3 rotationMatrix;
    rotationMatrix[0] = toTarget;
    rotationMatrix[1] = up;
    rotationMatrix[2] = right;

    glm::quat targetRotation = glm::quat_cast(rotationMatrix);

    Transform transform = bone->getLocalTransform();
    transform.rotation = glm::slerp(transform.rotation, targetRotation, weight_);
    bone->setLocalTransform(transform);

    skeleton->update();
}

}
}