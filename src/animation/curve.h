#pragma once

#include <vector>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace v3d {
namespace animation {

enum class InterpolationMode {
    Constant,
    Linear,
    Cubic,
    Step
};

enum class TangentMode {
    Auto,
    Linear,
    Constant,
    Free
};

template<typename T>
struct Keyframe {
    float time;
    T value;
    T inTangent;
    T outTangent;
    InterpolationMode interpolation;
    TangentMode tangentMode;

    Keyframe()
        : time(0.0f)
        , interpolation(InterpolationMode::Linear)
        , tangentMode(TangentMode::Auto) {
    }

    Keyframe(float t, const T& v)
        : time(t)
        , value(v)
        , interpolation(InterpolationMode::Linear)
        , tangentMode(TangentMode::Auto) {
    }

    Keyframe(float t, const T& v, InterpolationMode interp)
        : time(t)
        , value(v)
        , interpolation(interp)
        , tangentMode(TangentMode::Auto) {
    }
};

template<typename T>
class AnimationCurve {
public:
    AnimationCurve();

    void addKeyframe(const Keyframe<T>& keyframe);
    void removeKeyframe(int index);
    void clearKeyframes();

    const Keyframe<T>& getKeyframe(int index) const;
    Keyframe<T>& getKeyframe(int index);

    int getKeyframeCount() const;
    const std::vector<Keyframe<T>>& getKeyframes() const;

    T evaluate(float time) const;
    T evaluateLinear(float time) const;
    T evaluateCubic(float time) const;

    float getStartTime() const;
    float getEndTime() const;
    float getDuration() const;

    void sortKeyframes();
    void optimize();

    void setPreExtrapolationMode(InterpolationMode mode);
    void setPostExtrapolationMode(InterpolationMode mode);

    InterpolationMode getPreExtrapolationMode() const;
    InterpolationMode getPostExtrapolationMode() const;

private:
    int findKeyframeIndex(float time) const;
    T interpolateLinear(const Keyframe<T>& a, const Keyframe<T>& b, float t) const;
    T interpolateCubic(const Keyframe<T>& a, const Keyframe<T>& b, float t) const;

    std::vector<Keyframe<T>> keyframes_;
    InterpolationMode preExtrapolation_;
    InterpolationMode postExtrapolation_;
};

using FloatCurve = AnimationCurve<float>;
using Vec3Curve = AnimationCurve<glm::vec3>;
using QuatCurve = AnimationCurve<glm::quat>;

class CurveEditor {
public:
    static void calculateAutoTangents(std::vector<Keyframe<float>>& keyframes);
    static void calculateAutoTangents(std::vector<Keyframe<glm::vec3>>& keyframes);
    static void calculateAutoTangents(std::vector<Keyframe<glm::quat>>& keyframes);

    static void smoothCurve(std::vector<Keyframe<float>>& keyframes, float tension = 0.5f);
    static void smoothCurve(std::vector<Keyframe<glm::vec3>>& keyframes, float tension = 0.5f);

    static void resampleCurve(AnimationCurve<float>& curve, float interval);
    static void resampleCurve(AnimationCurve<glm::vec3>& curve, float interval);
    static void resampleCurve(AnimationCurve<glm::quat>& curve, float interval);

    static void compressCurve(AnimationCurve<float>& curve, float tolerance = 0.001f);
    static void compressCurve(AnimationCurve<glm::vec3>& curve, float tolerance = 0.001f);
};

}
}