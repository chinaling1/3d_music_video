#include "animation_controller.h"

namespace v3d {
namespace animation {

AnimationController::AnimationController()
    : skeleton_(nullptr)
    , globalSpeed_(1.0f)
    , playing_(false)
    , paused_(false) {
}

AnimationController::~AnimationController() {
}

void AnimationController::setSkeleton(const Skeleton* skeleton) {
    skeleton_ = skeleton;
}

const Skeleton* AnimationController::getSkeleton() const {
    return skeleton_;
}

void AnimationController::setStateMachine(std::shared_ptr<AnimationStateMachine> stateMachine) {
    stateMachine_ = stateMachine;
    if (stateMachine_ && skeleton_) {
        stateMachine_->setSkeleton(skeleton_);
    }
}

std::shared_ptr<AnimationStateMachine> AnimationController::getStateMachine() const {
    return stateMachine_;
}

void AnimationController::update(float deltaTime) {
    if (!playing_ || paused_ || !stateMachine_) {
        return;
    }

    float scaledDelta = deltaTime * globalSpeed_;
    stateMachine_->update(scaledDelta);
}

Pose AnimationController::getCurrentPose() const {
    if (stateMachine_) {
        return stateMachine_->getCurrentPose();
    }
    return Pose();
}

void AnimationController::play(const std::string& stateName) {
    if (stateMachine_) {
        stateMachine_->setCurrentState(stateName);
    }
    playing_ = true;
    paused_ = false;
}

void AnimationController::stop() {
    playing_ = false;
    paused_ = false;
}

void AnimationController::pause() {
    paused_ = true;
}

void AnimationController::resume() {
    paused_ = false;
}

void AnimationController::setSpeed(float speed) {
    globalSpeed_ = std::max(0.0f, speed);
}

float AnimationController::getSpeed() const {
    return globalSpeed_;
}

bool AnimationController::isPlaying() const {
    return playing_;
}

bool AnimationController::isPaused() const {
    return paused_;
}

}
}