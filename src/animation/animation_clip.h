#pragma once

#include "curve.h"
#include "skeleton.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace v3d {
namespace animation {

class AnimationClip {
public:
    AnimationClip();
    explicit AnimationClip(const std::string& name);

    std::string getName() const;
    void setName(const std::string& name);

    float getDuration() const;
    void setDuration(float duration);

    float getSampleRate() const;
    void setSampleRate(float sampleRate);

    bool isLooping() const;
    void setLooping(bool looping);

    void addPositionCurve(const std::string& boneName, std::shared_ptr<Vec3Curve> curve);
    void addRotationCurve(const std::string& boneName, std::shared_ptr<QuatCurve> curve);
    void addScaleCurve(const std::string& boneName, std::shared_ptr<Vec3Curve> curve);

    std::shared_ptr<Vec3Curve> getPositionCurve(const std::string& boneName) const;
    std::shared_ptr<QuatCurve> getRotationCurve(const std::string& boneName) const;
    std::shared_ptr<Vec3Curve> getScaleCurve(const std::string& boneName) const;

    void removeBoneCurves(const std::string& boneName);
    void clearAllCurves();

    std::vector<std::string> getAnimatedBones() const;

    void evaluate(float time, Pose& pose) const;
    Pose evaluate(float time) const;

    void optimize(float tolerance = 0.001f);
    void resample(float interval = 0.033f);

    void setRootMotionEnabled(bool enabled);
    bool isRootMotionEnabled() const;

    std::shared_ptr<Vec3Curve> getRootPositionCurve() const;
    std::shared_ptr<QuatCurve> getRootRotationCurve() const;

    void setRootPositionCurve(std::shared_ptr<Vec3Curve> curve);
    void setRootRotationCurve(std::shared_ptr<QuatCurve> curve);

private:
    std::string name_;
    float duration_;
    float sampleRate_;
    bool looping_;
    bool rootMotionEnabled_;

    std::unordered_map<std::string, std::shared_ptr<Vec3Curve>> positionCurves_;
    std::unordered_map<std::string, std::shared_ptr<QuatCurve>> rotationCurves_;
    std::unordered_map<std::string, std::shared_ptr<Vec3Curve>> scaleCurves_;

    std::shared_ptr<Vec3Curve> rootPositionCurve_;
    std::shared_ptr<QuatCurve> rootRotationCurve_;
};

class AnimationClipLibrary {
public:
    static AnimationClipLibrary& getInstance();

    std::shared_ptr<AnimationClip> createClip(const std::string& name);
    std::shared_ptr<AnimationClip> getClip(const std::string& name) const;

    void addClip(std::shared_ptr<AnimationClip> clip);
    void removeClip(const std::string& name);
    void clear();

    std::vector<std::string> getClipNames() const;
    size_t getClipCount() const;

    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath) const;

private:
    AnimationClipLibrary();

    std::unordered_map<std::string, std::shared_ptr<AnimationClip>> clips_;
};

}
}