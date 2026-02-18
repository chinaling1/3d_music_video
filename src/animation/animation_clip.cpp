#include "animation_clip.h"
#include <algorithm>

namespace v3d {
namespace animation {

AnimationClip::AnimationClip()
    : duration_(0.0f)
    , sampleRate_(30.0f)
    , looping_(false)
    , rootMotionEnabled_(false) {
}

AnimationClip::AnimationClip(const std::string& name)
    : name_(name)
    , duration_(0.0f)
    , sampleRate_(30.0f)
    , looping_(false)
    , rootMotionEnabled_(false) {
}

std::string AnimationClip::getName() const {
    return name_;
}

void AnimationClip::setName(const std::string& name) {
    name_ = name;
}

float AnimationClip::getDuration() const {
    return duration_;
}

void AnimationClip::setDuration(float duration) {
    duration_ = std::max(0.0f, duration);
}

float AnimationClip::getSampleRate() const {
    return sampleRate_;
}

void AnimationClip::setSampleRate(float sampleRate) {
    sampleRate_ = std::max(1.0f, sampleRate);
}

bool AnimationClip::isLooping() const {
    return looping_;
}

void AnimationClip::setLooping(bool looping) {
    looping_ = looping;
}

void AnimationClip::addPositionCurve(const std::string& boneName, std::shared_ptr<Vec3Curve> curve) {
    positionCurves_[boneName] = curve;
}

void AnimationClip::addRotationCurve(const std::string& boneName, std::shared_ptr<QuatCurve> curve) {
    rotationCurves_[boneName] = curve;
}

void AnimationClip::addScaleCurve(const std::string& boneName, std::shared_ptr<Vec3Curve> curve) {
    scaleCurves_[boneName] = curve;
}

std::shared_ptr<Vec3Curve> AnimationClip::getPositionCurve(const std::string& boneName) const {
    auto it = positionCurves_.find(boneName);
    return it != positionCurves_.end() ? it->second : nullptr;
}

std::shared_ptr<QuatCurve> AnimationClip::getRotationCurve(const std::string& boneName) const {
    auto it = rotationCurves_.find(boneName);
    return it != rotationCurves_.end() ? it->second : nullptr;
}

std::shared_ptr<Vec3Curve> AnimationClip::getScaleCurve(const std::string& boneName) const {
    auto it = scaleCurves_.find(boneName);
    return it != scaleCurves_.end() ? it->second : nullptr;
}

void AnimationClip::removeBoneCurves(const std::string& boneName) {
    positionCurves_.erase(boneName);
    rotationCurves_.erase(boneName);
    scaleCurves_.erase(boneName);
}

void AnimationClip::clearAllCurves() {
    positionCurves_.clear();
    rotationCurves_.clear();
    scaleCurves_.clear();
    rootPositionCurve_.reset();
    rootRotationCurve_.reset();
}

std::vector<std::string> AnimationClip::getAnimatedBones() const {
    std::vector<std::string> bones;

    for (const auto& pair : positionCurves_) {
        if (std::find(bones.begin(), bones.end(), pair.first) == bones.end()) {
            bones.push_back(pair.first);
        }
    }

    for (const auto& pair : rotationCurves_) {
        if (std::find(bones.begin(), bones.end(), pair.first) == bones.end()) {
            bones.push_back(pair.first);
        }
    }

    for (const auto& pair : scaleCurves_) {
        if (std::find(bones.begin(), bones.end(), pair.first) == bones.end()) {
            bones.push_back(pair.first);
        }
    }

    return bones;
}

void AnimationClip::evaluate(float time, Pose& pose) const {
    if (!pose.getSkeleton()) {
        return;
    }

    float evalTime = time;
    if (looping_ && duration_ > 0.0f) {
        evalTime = fmod(time, duration_);
    } else if (time > duration_) {
        evalTime = duration_;
    }

    for (const auto& pair : positionCurves_) {
        if (auto* bone = pose.getSkeleton()->getBone(pair.first)) {
            int index = bone->getIndex();
            glm::vec3 position = pair.second->evaluate(evalTime);
            Transform transform = pose.getLocalTransform(index);
            transform.position = position;
            pose.setLocalTransform(index, transform);
        }
    }

    for (const auto& pair : rotationCurves_) {
        if (auto* bone = pose.getSkeleton()->getBone(pair.first)) {
            int index = bone->getIndex();
            glm::quat rotation = pair.second->evaluate(evalTime);
            Transform transform = pose.getLocalTransform(index);
            transform.rotation = rotation;
            pose.setLocalTransform(index, transform);
        }
    }

    for (const auto& pair : scaleCurves_) {
        if (auto* bone = pose.getSkeleton()->getBone(pair.first)) {
            int index = bone->getIndex();
            glm::vec3 scale = pair.second->evaluate(evalTime);
            Transform transform = pose.getLocalTransform(index);
            transform.scale = scale;
            pose.setLocalTransform(index, transform);
        }
    }

    pose.calculateWorldTransforms();
}

Pose AnimationClip::evaluate(float time) const {
    Pose result;
    return result;
}

void AnimationClip::optimize(float tolerance) {
    for (auto& pair : positionCurves_) {
        pair.second->optimize();
    }

    for (auto& pair : rotationCurves_) {
        pair.second->optimize();
    }

    for (auto& pair : scaleCurves_) {
        pair.second->optimize();
    }

    if (rootPositionCurve_) {
        rootPositionCurve_->optimize();
    }

    if (rootRotationCurve_) {
        rootRotationCurve_->optimize();
    }
}

void AnimationClip::resample(float interval) {
    for (auto& pair : positionCurves_) {
        CurveEditor::resampleCurve(*pair.second, interval);
    }

    for (auto& pair : rotationCurves_) {
        CurveEditor::resampleCurve(*pair.second, interval);
    }

    for (auto& pair : scaleCurves_) {
        CurveEditor::resampleCurve(*pair.second, interval);
    }

    if (rootPositionCurve_) {
        CurveEditor::resampleCurve(*rootPositionCurve_, interval);
    }

    if (rootRotationCurve_) {
        CurveEditor::resampleCurve(*rootRotationCurve_, interval);
    }
}

void AnimationClip::setRootMotionEnabled(bool enabled) {
    rootMotionEnabled_ = enabled;
}

bool AnimationClip::isRootMotionEnabled() const {
    return rootMotionEnabled_;
}

std::shared_ptr<Vec3Curve> AnimationClip::getRootPositionCurve() const {
    return rootPositionCurve_;
}

std::shared_ptr<QuatCurve> AnimationClip::getRootRotationCurve() const {
    return rootRotationCurve_;
}

void AnimationClip::setRootPositionCurve(std::shared_ptr<Vec3Curve> curve) {
    rootPositionCurve_ = curve;
}

void AnimationClip::setRootRotationCurve(std::shared_ptr<QuatCurve> curve) {
    rootRotationCurve_ = curve;
}

AnimationClipLibrary& AnimationClipLibrary::getInstance() {
    static AnimationClipLibrary instance;
    return instance;
}

AnimationClipLibrary::AnimationClipLibrary() {
}

std::shared_ptr<AnimationClip> AnimationClipLibrary::createClip(const std::string& name) {
    auto clip = std::make_shared<AnimationClip>(name);
    clips_[name] = clip;
    return clip;
}

std::shared_ptr<AnimationClip> AnimationClipLibrary::getClip(const std::string& name) const {
    auto it = clips_.find(name);
    return it != clips_.end() ? it->second : nullptr;
}

void AnimationClipLibrary::addClip(std::shared_ptr<AnimationClip> clip) {
    if (clip) {
        clips_[clip->getName()] = clip;
    }
}

void AnimationClipLibrary::removeClip(const std::string& name) {
    clips_.erase(name);
}

void AnimationClipLibrary::clear() {
    clips_.clear();
}

std::vector<std::string> AnimationClipLibrary::getClipNames() const {
    std::vector<std::string> names;
    for (const auto& pair : clips_) {
        names.push_back(pair.first);
    }
    return names;
}

size_t AnimationClipLibrary::getClipCount() const {
    return clips_.size();
}

bool AnimationClipLibrary::loadFromFile(const std::string& filePath) {
    return false;
}

bool AnimationClipLibrary::saveToFile(const std::string& filePath) const {
    return false;
}

}
}