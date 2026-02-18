#include "curve.h"
#include <algorithm>
#include <cmath>

namespace v3d {
namespace animation {

template<typename T>
AnimationCurve<T>::AnimationCurve()
    : preExtrapolation_(InterpolationMode::Constant)
    , postExtrapolation_(InterpolationMode::Constant) {
}

template<typename T>
void AnimationCurve<T>::addKeyframe(const Keyframe<T>& keyframe) {
    keyframes_.push_back(keyframe);
    sortKeyframes();
}

template<typename T>
void AnimationCurve<T>::removeKeyframe(int index) {
    if (index >= 0 && index < static_cast<int>(keyframes_.size())) {
        keyframes_.erase(keyframes_.begin() + index);
    }
}

template<typename T>
void AnimationCurve<T>::clearKeyframes() {
    keyframes_.clear();
}

template<typename T>
const Keyframe<T>& AnimationCurve<T>::getKeyframe(int index) const {
    return keyframes_[index];
}

template<typename T>
Keyframe<T>& AnimationCurve<T>::getKeyframe(int index) {
    return keyframes_[index];
}

template<typename T>
int AnimationCurve<T>::getKeyframeCount() const {
    return static_cast<int>(keyframes_.size());
}

template<typename T>
const std::vector<Keyframe<T>>& AnimationCurve<T>::getKeyframes() const {
    return keyframes_;
}

template<typename T>
T AnimationCurve<T>::evaluate(float time) const {
    if (keyframes_.empty()) {
        return T();
    }

    if (keyframes_.size() == 1) {
        return keyframes_[0].value;
    }

    float startTime = getStartTime();
    float endTime = getEndTime();

    if (time <= startTime) {
        switch (preExtrapolation_) {
            case InterpolationMode::Constant:
                return keyframes_[0].value;
            case InterpolationMode::Linear:
                return evaluateLinear(time);
            default:
                return keyframes_[0].value;
        }
    }

    if (time >= endTime) {
        switch (postExtrapolation_) {
            case InterpolationMode::Constant:
                return keyframes_.back().value;
            case InterpolationMode::Linear:
                return evaluateLinear(time);
            default:
                return keyframes_.back().value;
        }
    }

    int index = findKeyframeIndex(time);

    if (index < 0) {
        return keyframes_[0].value;
    }

    if (index >= static_cast<int>(keyframes_.size()) - 1) {
        return keyframes_.back().value;
    }

    const auto& a = keyframes_[index];
    const auto& b = keyframes_[index + 1];

    if (a.interpolation == InterpolationMode::Constant) {
        return a.value;
    }

    if (a.interpolation == InterpolationMode::Step) {
        return a.value;
    }

    float t = (time - a.time) / (b.time - a.time);

    if (a.interpolation == InterpolationMode::Linear) {
        return interpolateLinear(a, b, t);
    }

    if (a.interpolation == InterpolationMode::Cubic) {
        return interpolateCubic(a, b, t);
    }

    return a.value;
}

template<typename T>
T AnimationCurve<T>::evaluateLinear(float time) const {
    if (keyframes_.size() < 2) {
        return keyframes_.empty() ? T() : keyframes_[0].value;
    }

    int index = findKeyframeIndex(time);
    if (index < 0) index = 0;
    if (index >= static_cast<int>(keyframes_.size()) - 1) {
        index = static_cast<int>(keyframes_.size()) - 2;
    }

    const auto& a = keyframes_[index];
    const auto& b = keyframes_[index + 1];

    float t = (time - a.time) / (b.time - a.time);
    return interpolateLinear(a, b, t);
}

template<typename T>
T AnimationCurve<T>::evaluateCubic(float time) const {
    if (keyframes_.size() < 2) {
        return keyframes_.empty() ? T() : keyframes_[0].value;
    }

    int index = findKeyframeIndex(time);
    if (index < 0) index = 0;
    if (index >= static_cast<int>(keyframes_.size()) - 1) {
        index = static_cast<int>(keyframes_.size()) - 2;
    }

    const auto& a = keyframes_[index];
    const auto& b = keyframes_[index + 1];

    float t = (time - a.time) / (b.time - a.time);
    return interpolateCubic(a, b, t);
}

template<typename T>
float AnimationCurve<T>::getStartTime() const {
    return keyframes_.empty() ? 0.0f : keyframes_.front().time;
}

template<typename T>
float AnimationCurve<T>::getEndTime() const {
    return keyframes_.empty() ? 0.0f : keyframes_.back().time;
}

template<typename T>
float AnimationCurve<T>::getDuration() const {
    return getEndTime() - getStartTime();
}

template<typename T>
void AnimationCurve<T>::sortKeyframes() {
    std::sort(keyframes_.begin(), keyframes_.end(),
        [](const Keyframe<T>& a, const Keyframe<T>& b) {
            return a.time < b.time;
        });
}

template<typename T>
void AnimationCurve<T>::optimize() {
    if (keyframes_.size() < 3) {
        return;
    }

    std::vector<Keyframe<T>> optimized;
    optimized.push_back(keyframes_[0]);

    for (size_t i = 1; i < keyframes_.size() - 1; ++i) {
        const auto& prev = keyframes_[i - 1];
        const auto& curr = keyframes_[i];
        const auto& next = keyframes_[i + 1];

        T interpolated = interpolateLinear(prev, next, 0.5f);

        if (glm::length(interpolated - curr.value) > 0.0001f) {
            optimized.push_back(curr);
        }
    }

    optimized.push_back(keyframes_.back());
    keyframes_ = std::move(optimized);
}

template<typename T>
void AnimationCurve<T>::setPreExtrapolationMode(InterpolationMode mode) {
    preExtrapolation_ = mode;
}

template<typename T>
void AnimationCurve<T>::setPostExtrapolationMode(InterpolationMode mode) {
    postExtrapolation_ = mode;
}

template<typename T>
InterpolationMode AnimationCurve<T>::getPreExtrapolationMode() const {
    return preExtrapolation_;
}

template<typename T>
InterpolationMode AnimationCurve<T>::getPostExtrapolationMode() const {
    return postExtrapolation_;
}

template<typename T>
int AnimationCurve<T>::findKeyframeIndex(float time) const {
    if (keyframes_.empty()) {
        return -1;
    }

    if (time <= keyframes_[0].time) {
        return 0;
    }

    if (time >= keyframes_.back().time) {
        return static_cast<int>(keyframes_.size()) - 1;
    }

    auto it = std::lower_bound(keyframes_.begin(), keyframes_.end(), time,
        [](const Keyframe<T>& kf, float t) {
            return kf.time < t;
        });

    return static_cast<int>(std::distance(keyframes_.begin(), it)) - 1;
}

template<typename T>
T AnimationCurve<T>::interpolateLinear(const Keyframe<T>& a, const Keyframe<T>& b, float t) const {
    return glm::mix(a.value, b.value, t);
}

template<typename T>
T AnimationCurve<T>::interpolateCubic(const Keyframe<T>& a, const Keyframe<T>& b, float t) const {
    float t2 = t * t;
    float t3 = t2 * t;

    T p0 = a.value;
    T p1 = a.value + a.outTangent * (b.time - a.time) / 3.0f;
    T p2 = b.value - b.inTangent * (b.time - a.time) / 3.0f;
    T p3 = b.value;

    return ((1.0f - 3.0f * t2 + 2.0f * t3) * p0 +
            (t - 2.0f * t2 + t3) * p1 +
            (t2 - t3) * p2 +
            (3.0f * t2 - 2.0f * t3) * p3);
}

template<>
glm::quat AnimationCurve<glm::quat>::interpolateLinear(const Keyframe<glm::quat>& a, const Keyframe<glm::quat>& b, float t) const {
    return glm::slerp(a.value, b.value, t);
}

template<>
glm::quat AnimationCurve<glm::quat>::interpolateCubic(const Keyframe<glm::quat>& a, const Keyframe<glm::quat>& b, float t) const {
    return glm::slerp(a.value, b.value, t);
}

void CurveEditor::calculateAutoTangents(std::vector<Keyframe<float>>& keyframes) {
    if (keyframes.size() < 2) {
        return;
    }

    for (size_t i = 0; i < keyframes.size(); ++i) {
        if (i == 0) {
            keyframes[i].outTangent = (keyframes[i + 1].value - keyframes[i].value) / (keyframes[i + 1].time - keyframes[i].time);
            keyframes[i].inTangent = keyframes[i].outTangent;
        } else if (i == keyframes.size() - 1) {
            keyframes[i].inTangent = (keyframes[i].value - keyframes[i - 1].value) / (keyframes[i].time - keyframes[i - 1].time);
            keyframes[i].outTangent = keyframes[i].inTangent;
        } else {
            float prevSlope = (keyframes[i].value - keyframes[i - 1].value) / (keyframes[i].time - keyframes[i - 1].time);
            float nextSlope = (keyframes[i + 1].value - keyframes[i].value) / (keyframes[i + 1].time - keyframes[i].time);
            keyframes[i].inTangent = (prevSlope + nextSlope) * 0.5f;
            keyframes[i].outTangent = keyframes[i].inTangent;
        }
    }
}

void CurveEditor::calculateAutoTangents(std::vector<Keyframe<glm::vec3>>& keyframes) {
    if (keyframes.size() < 2) {
        return;
    }

    for (size_t i = 0; i < keyframes.size(); ++i) {
        if (i == 0) {
            keyframes[i].outTangent = (keyframes[i + 1].value - keyframes[i].value) / (keyframes[i + 1].time - keyframes[i].time);
            keyframes[i].inTangent = keyframes[i].outTangent;
        } else if (i == keyframes.size() - 1) {
            keyframes[i].inTangent = (keyframes[i].value - keyframes[i - 1].value) / (keyframes[i].time - keyframes[i - 1].time);
            keyframes[i].outTangent = keyframes[i].inTangent;
        } else {
            glm::vec3 prevSlope = (keyframes[i].value - keyframes[i - 1].value) / (keyframes[i].time - keyframes[i - 1].time);
            glm::vec3 nextSlope = (keyframes[i + 1].value - keyframes[i].value) / (keyframes[i + 1].time - keyframes[i].time);
            keyframes[i].inTangent = (prevSlope + nextSlope) * 0.5f;
            keyframes[i].outTangent = keyframes[i].inTangent;
        }
    }
}

void CurveEditor::calculateAutoTangents(std::vector<Keyframe<glm::quat>>& keyframes) {
}

void CurveEditor::smoothCurve(std::vector<Keyframe<float>>& keyframes, float tension) {
    if (keyframes.size() < 3) {
        return;
    }

    std::vector<float> newValues(keyframes.size());

    for (size_t i = 1; i < keyframes.size() - 1; ++i) {
        float prev = keyframes[i - 1].value;
        float curr = keyframes[i].value;
        float next = keyframes[i + 1].value;

        newValues[i] = curr + tension * (prev - 2.0f * curr + next) * 0.5f;
    }

    for (size_t i = 1; i < keyframes.size() - 1; ++i) {
        keyframes[i].value = newValues[i];
    }
}

void CurveEditor::smoothCurve(std::vector<Keyframe<glm::vec3>>& keyframes, float tension) {
    if (keyframes.size() < 3) {
        return;
    }

    std::vector<glm::vec3> newValues(keyframes.size());

    for (size_t i = 1; i < keyframes.size() - 1; ++i) {
        glm::vec3 prev = keyframes[i - 1].value;
        glm::vec3 curr = keyframes[i].value;
        glm::vec3 next = keyframes[i + 1].value;

        newValues[i] = curr + tension * (prev - 2.0f * curr + next) * 0.5f;
    }

    for (size_t i = 1; i < keyframes.size() - 1; ++i) {
        keyframes[i].value = newValues[i];
    }
}

void CurveEditor::resampleCurve(AnimationCurve<float>& curve, float interval) {
    std::vector<Keyframe<float>> newKeyframes;

    float startTime = curve.getStartTime();
    float endTime = curve.getEndTime();

    for (float t = startTime; t <= endTime; t += interval) {
        Keyframe<float> kf;
        kf.time = t;
        kf.value = curve.evaluate(t);
        kf.interpolation = InterpolationMode::Linear;
        newKeyframes.push_back(kf);
    }

    curve.clearKeyframes();
    for (const auto& kf : newKeyframes) {
        curve.addKeyframe(kf);
    }
}

void CurveEditor::resampleCurve(AnimationCurve<glm::vec3>& curve, float interval) {
    std::vector<Keyframe<glm::vec3>> newKeyframes;

    float startTime = curve.getStartTime();
    float endTime = curve.getEndTime();

    for (float t = startTime; t <= endTime; t += interval) {
        Keyframe<glm::vec3> kf;
        kf.time = t;
        kf.value = curve.evaluate(t);
        kf.interpolation = InterpolationMode::Linear;
        newKeyframes.push_back(kf);
    }

    curve.clearKeyframes();
    for (const auto& kf : newKeyframes) {
        curve.addKeyframe(kf);
    }
}

void CurveEditor::resampleCurve(AnimationCurve<glm::quat>& curve, float interval) {
    std::vector<Keyframe<glm::quat>> newKeyframes;

    float startTime = curve.getStartTime();
    float endTime = curve.getEndTime();

    for (float t = startTime; t <= endTime; t += interval) {
        Keyframe<glm::quat> kf;
        kf.time = t;
        kf.value = curve.evaluate(t);
        kf.interpolation = InterpolationMode::Linear;
        newKeyframes.push_back(kf);
    }

    curve.clearKeyframes();
    for (const auto& kf : newKeyframes) {
        curve.addKeyframe(kf);
    }
}

void CurveEditor::compressCurve(AnimationCurve<float>& curve, float tolerance) {
    curve.optimize();
}

void CurveEditor::compressCurve(AnimationCurve<glm::vec3>& curve, float tolerance) {
    curve.optimize();
}

}
}