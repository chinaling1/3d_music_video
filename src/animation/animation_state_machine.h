#pragma once

#include "skeleton.h"
#include "animation_clip.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace v3d {
namespace animation {

class AnimationState {
public:
    AnimationState();
    explicit AnimationState(const std::string& name);

    std::string getName() const;
    void setName(const std::string& name);

    void setAnimationClip(std::shared_ptr<AnimationClip> clip);
    std::shared_ptr<AnimationClip> getAnimationClip() const;

    float getSpeed() const;
    void setSpeed(float speed);

    float getBlendWeight() const;
    void setBlendWeight(float weight);

    bool isLooping() const;
    void setLooping(bool looping);

    void setTime(float time);
    float getTime() const;

    void update(float deltaTime);
    Pose getCurrentPose() const;

    void setOnCompleteCallback(std::function<void()> callback);

private:
    std::string name_;
    std::shared_ptr<AnimationClip> clip_;
    float speed_;
    float blendWeight_;
    bool looping_;
    float currentTime_;
    std::function<void()> onCompleteCallback_;
    bool completed_;
};

class AnimationTransition {
public:
    AnimationTransition();

    std::string getSourceState() const;
    void setSourceState(const std::string& state);

    std::string getTargetState() const;
    void setTargetState(const std::string& state);

    float getDuration() const;
    void setDuration(float duration);

    bool hasExitTime() const;
    void setExitTime(float exitTime);

    float getExitTime() const;

    void addCondition(std::function<bool()> condition);
    bool evaluateConditions() const;

    void clearConditions();

private:
    std::string sourceState_;
    std::string targetState_;
    float duration_;
    bool hasExitTime_;
    float exitTime_;
    std::vector<std::function<bool()>> conditions_;
};

class AnimationStateMachine {
public:
    AnimationStateMachine();
    ~AnimationStateMachine();

    void setSkeleton(const Skeleton* skeleton);
    const Skeleton* getSkeleton() const;

    std::shared_ptr<AnimationState> createState(const std::string& name);
    std::shared_ptr<AnimationState> getState(const std::string& name) const;
    void removeState(const std::string& name);

    std::shared_ptr<AnimationTransition> createTransition(
        const std::string& sourceState,
        const std::string& targetState
    );

    void setDefaultState(const std::string& name);
    std::string getDefaultState() const;

    void setCurrentState(const std::string& name);
    std::string getCurrentState() const;

    void update(float deltaTime);

    Pose getCurrentPose() const;

    void setParameter(const std::string& name, float value);
    float getParameter(const std::string& name) const;
    bool hasParameter(const std::string& name) const;

    void setBoolParameter(const std::string& name, bool value);
    bool getBoolParameter(const std::string& name) const;

    void setTriggerParameter(const std::string& name);
    bool getTriggerParameter(const std::string& name) const;
    void resetTriggerParameter(const std::string& name);

    void clearParameters();

private:
    void evaluateTransitions();
    void performTransition(const std::shared_ptr<AnimationTransition>& transition);

    const Skeleton* skeleton_;
    std::unordered_map<std::string, std::shared_ptr<AnimationState>> states_;
    std::vector<std::shared_ptr<AnimationTransition>> transitions_;
    std::string defaultState_;
    std::string currentState_;
    std::string previousState_;
    float transitionTime_;
    std::shared_ptr<AnimationTransition> currentTransition_;
    Pose currentPose_;
    Pose previousPose_;

    std::unordered_map<std::string, float> floatParameters_;
    std::unordered_map<std::string, bool> boolParameters_;
    std::unordered_map<std::string, bool> triggerParameters_;
};

}
}