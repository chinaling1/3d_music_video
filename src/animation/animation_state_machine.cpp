#include "animation_state_machine.h"
#include <algorithm>

namespace v3d {
namespace animation {

AnimationState::AnimationState()
    : speed_(1.0f)
    , blendWeight_(1.0f)
    , looping_(true)
    , currentTime_(0.0f)
    , completed_(false) {
}

AnimationState::AnimationState(const std::string& name)
    : name_(name)
    , speed_(1.0f)
    , blendWeight_(1.0f)
    , looping_(true)
    , currentTime_(0.0f)
    , completed_(false) {
}

std::string AnimationState::getName() const {
    return name_;
}

void AnimationState::setName(const std::string& name) {
    name_ = name;
}

void AnimationState::setAnimationClip(std::shared_ptr<AnimationClip> clip) {
    clip_ = clip;
}

std::shared_ptr<AnimationClip> AnimationState::getAnimationClip() const {
    return clip_;
}

float AnimationState::getSpeed() const {
    return speed_;
}

void AnimationState::setSpeed(float speed) {
    speed_ = std::max(0.0f, speed);
}

float AnimationState::getBlendWeight() const {
    return blendWeight_;
}

void AnimationState::setBlendWeight(float weight) {
    blendWeight_ = std::clamp(weight, 0.0f, 1.0f);
}

bool AnimationState::isLooping() const {
    return looping_;
}

void AnimationState::setLooping(bool looping) {
    looping_ = looping;
}

void AnimationState::setTime(float time) {
    currentTime_ = std::max(0.0f, time);
}

float AnimationState::getTime() const {
    return currentTime_;
}

void AnimationState::update(float deltaTime) {
    if (!clip_) {
        return;
    }

    currentTime_ += deltaTime * speed_;

    if (currentTime_ >= clip_->getDuration()) {
        if (looping_) {
            currentTime_ = fmod(currentTime_, clip_->getDuration());
        } else {
            currentTime_ = clip_->getDuration();
            if (!completed_ && onCompleteCallback_) {
                onCompleteCallback_();
            }
            completed_ = true;
        }
    }
}

Pose AnimationState::getCurrentPose() const {
    Pose pose;
    if (clip_) {
        clip_->evaluate(currentTime_, pose);
    }
    return pose;
}

void AnimationState::setOnCompleteCallback(std::function<void()> callback) {
    onCompleteCallback_ = callback;
}

AnimationTransition::AnimationTransition()
    : duration_(0.3f)
    , hasExitTime_(false)
    , exitTime_(0.9f) {
}

std::string AnimationTransition::getSourceState() const {
    return sourceState_;
}

void AnimationTransition::setSourceState(const std::string& state) {
    sourceState_ = state;
}

std::string AnimationTransition::getTargetState() const {
    return targetState_;
}

void AnimationTransition::setTargetState(const std::string& state) {
    targetState_ = state;
}

float AnimationTransition::getDuration() const {
    return duration_;
}

void AnimationTransition::setDuration(float duration) {
    duration_ = std::max(0.0f, duration);
}

bool AnimationTransition::hasExitTime() const {
    return hasExitTime_;
}

void AnimationTransition::setExitTime(float exitTime) {
    hasExitTime_ = true;
    exitTime_ = std::clamp(exitTime, 0.0f, 1.0f);
}

float AnimationTransition::getExitTime() const {
    return exitTime_;
}

void AnimationTransition::addCondition(std::function<bool()> condition) {
    conditions_.push_back(condition);
}

bool AnimationTransition::evaluateConditions() const {
    for (const auto& condition : conditions_) {
        if (!condition()) {
            return false;
        }
    }
    return true;
}

void AnimationTransition::clearConditions() {
    conditions_.clear();
}

AnimationStateMachine::AnimationStateMachine()
    : skeleton_(nullptr)
    , transitionTime_(0.0f) {
}

AnimationStateMachine::~AnimationStateMachine() {
}

void AnimationStateMachine::setSkeleton(const Skeleton* skeleton) {
    skeleton_ = skeleton;
}

const Skeleton* AnimationStateMachine::getSkeleton() const {
    return skeleton_;
}

std::shared_ptr<AnimationState> AnimationStateMachine::createState(const std::string& name) {
    auto state = std::make_shared<AnimationState>(name);
    states_[name] = state;
    return state;
}

std::shared_ptr<AnimationState> AnimationStateMachine::getState(const std::string& name) const {
    auto it = states_.find(name);
    return it != states_.end() ? it->second : nullptr;
}

void AnimationStateMachine::removeState(const std::string& name) {
    states_.erase(name);
}

std::shared_ptr<AnimationTransition> AnimationStateMachine::createTransition(
    const std::string& sourceState,
    const std::string& targetState) {

    auto transition = std::make_shared<AnimationTransition>();
    transition->setSourceState(sourceState);
    transition->setTargetState(targetState);
    transitions_.push_back(transition);
    return transition;
}

void AnimationStateMachine::setDefaultState(const std::string& name) {
    defaultState_ = name;
    if (currentState_.empty()) {
        setCurrentState(name);
    }
}

std::string AnimationStateMachine::getDefaultState() const {
    return defaultState_;
}

void AnimationStateMachine::setCurrentState(const std::string& name) {
    if (currentState_ != name) {
        previousState_ = currentState_;
        currentState_ = name;
        transitionTime_ = 0.0f;
        currentTransition_ = nullptr;

        auto state = getState(currentState_);
        if (state) {
            state->setTime(0.0f);
        }
    }
}

std::string AnimationStateMachine::getCurrentState() const {
    return currentState_;
}

void AnimationStateMachine::update(float deltaTime) {
    if (currentState_.empty()) {
        return;
    }

    auto currentState = getState(currentState_);
    if (!currentState) {
        return;
    }

    currentState->update(deltaTime);

    if (currentTransition_) {
        transitionTime_ += deltaTime;

        if (transitionTime_ >= currentTransition_->getDuration()) {
            setCurrentState(currentTransition_->getTargetState());
        }
    } else {
        evaluateTransitions();
    }

    if (skeleton_) {
        currentPose_.setSkeleton(skeleton_);
    }

    if (currentTransition_) {
        auto previousState = getState(previousState_);
        if (previousState) {
            previousPose_ = previousState->getCurrentPose();
        }

        float blendFactor = transitionTime_ / currentTransition_->getDuration();
        currentPose_ = currentState->getCurrentPose();
        currentPose_.blend(previousPose_, blendFactor);
    } else {
        currentPose_ = currentState->getCurrentPose();
    }

    currentPose_.calculateWorldTransforms();
}

Pose AnimationStateMachine::getCurrentPose() const {
    return currentPose_;
}

void AnimationStateMachine::setParameter(const std::string& name, float value) {
    floatParameters_[name] = value;
}

float AnimationStateMachine::getParameter(const std::string& name) const {
    auto it = floatParameters_.find(name);
    return it != floatParameters_.end() ? it->second : 0.0f;
}

bool AnimationStateMachine::hasParameter(const std::string& name) const {
    return floatParameters_.find(name) != floatParameters_.end() ||
           boolParameters_.find(name) != boolParameters_.end() ||
           triggerParameters_.find(name) != triggerParameters_.end();
}

void AnimationStateMachine::setBoolParameter(const std::string& name, bool value) {
    boolParameters_[name] = value;
}

bool AnimationStateMachine::getBoolParameter(const std::string& name) const {
    auto it = boolParameters_.find(name);
    return it != boolParameters_.end() ? it->second : false;
}

void AnimationStateMachine::setTriggerParameter(const std::string& name) {
    triggerParameters_[name] = true;
}

bool AnimationStateMachine::getTriggerParameter(const std::string& name) const {
    auto it = triggerParameters_.find(name);
    return it != triggerParameters_.end() ? it->second : false;
}

void AnimationStateMachine::resetTriggerParameter(const std::string& name) {
    triggerParameters_[name] = false;
}

void AnimationStateMachine::clearParameters() {
    floatParameters_.clear();
    boolParameters_.clear();
    triggerParameters_.clear();
}

void AnimationStateMachine::evaluateTransitions() {
    auto currentState = getState(currentState_);
    if (!currentState || !currentState->getAnimationClip()) {
        return;
    }

    float clipDuration = currentState->getAnimationClip()->getDuration();
    float currentTime = currentState->getTime();
    float normalizedTime = clipDuration > 0.0f ? currentTime / clipDuration : 0.0f;

    for (const auto& transition : transitions_) {
        if (transition->getSourceState() != currentState_) {
            continue;
        }

        if (transition->hasExitTime() && normalizedTime < transition->getExitTime()) {
            continue;
        }

        if (transition->evaluateConditions()) {
            performTransition(transition);
            break;
        }
    }
}

void AnimationStateMachine::performTransition(const std::shared_ptr<AnimationTransition>& transition) {
    currentTransition_ = transition;
    transitionTime_ = 0.0f;
}

}
}