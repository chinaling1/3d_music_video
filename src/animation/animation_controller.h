#pragma once

#include "animation_state_machine.h"

namespace v3d {
namespace animation {

class AnimationController {
public:
    AnimationController();
    ~AnimationController();

    void setSkeleton(const Skeleton* skeleton);
    const Skeleton* getSkeleton() const;

    void setStateMachine(std::shared_ptr<AnimationStateMachine> stateMachine);
    std::shared_ptr<AnimationStateMachine> getStateMachine() const;

    void update(float deltaTime);

    Pose getCurrentPose() const;

    void play(const std::string& stateName);
    void stop();
    void pause();
    void resume();

    void setSpeed(float speed);
    float getSpeed() const;

    bool isPlaying() const;
    bool isPaused() const;

private:
    const Skeleton* skeleton_;
    std::shared_ptr<AnimationStateMachine> stateMachine_;
    float globalSpeed_;
    bool playing_;
    bool paused_;
};

}
}