/**
 * @file lighting_system.h
 * @brief 光照系统 - 管理3D场景中的光源
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace v3d {
namespace modeling {

enum class LightType {
    Directional,
    Point,
    Spot,
    Area,
    Hemisphere,
    Sun
};

struct ShadowSettings {
    bool enabled;
    int resolution;
    float bias;
    float normalBias;
    float nearPlane;
    float farPlane;
    int cascadeCount;
    float cascadeSplitFactor;
    float softness;
    int sampleCount;
    
    ShadowSettings()
        : enabled(true), resolution(1024), bias(0.001f), normalBias(0.02f)
        , nearPlane(0.1f), farPlane(100.0f), cascadeCount(4)
        , cascadeSplitFactor(0.5f), softness(1.0f), sampleCount(16) {}
};

struct Light {
    std::string id;
    std::string name;
    LightType type;
    
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    
    glm::vec3 color;
    float intensity;
    float power;
    
    float radius;
    float falloff;
    
    float innerConeAngle;
    float outerConeAngle;
    float spotBlend;
    
    glm::vec2 areaSize;
    int areaShape;
    
    bool castShadows;
    ShadowSettings shadowSettings;
    
    bool affectDiffuse;
    bool affectSpecular;
    bool visible;
    
    float volumetricIntensity;
    float volumetricDistance;
    
    std::vector<std::string> excludeObjects;
    
    Light();
    
    glm::vec3 getDirection() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio = 1.0f) const;
    glm::mat4 getViewProjectionMatrix(float aspectRatio = 1.0f) const;
    
    void setType(LightType newType);
    void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
};

struct LightProbe {
    std::string id;
    std::string name;
    glm::vec3 position;
    glm::vec3 size;
    float intensity;
    bool captureScene;
    std::string cubemapPath;
    
    LightProbe() : intensity(1.0f), captureScene(true) {}
};

class LightingSystem {
public:
    static LightingSystem& getInstance();
    
    std::shared_ptr<Light> createLight(LightType type = LightType::Point, const std::string& name = "");
    std::shared_ptr<Light> getLight(const std::string& id) const;
    void removeLight(const std::string& id);
    std::vector<std::shared_ptr<Light>> getAllLights() const;
    std::vector<std::shared_ptr<Light>> getLightsByType(LightType type) const;
    
    std::shared_ptr<LightProbe> createLightProbe(const std::string& name = "");
    std::shared_ptr<LightProbe> getLightProbe(const std::string& id) const;
    void removeLightProbe(const std::string& id);
    std::vector<std::shared_ptr<LightProbe>> getAllLightProbes() const;
    
    void setAmbientColor(const glm::vec3& color);
    glm::vec3 getAmbientColor() const { return ambientColor_; }
    
    void setAmbientIntensity(float intensity);
    float getAmbientIntensity() const { return ambientIntensity_; }
    
    void setEnvironmentMap(const std::string& path);
    std::string getEnvironmentMap() const { return environmentMap_; }
    
    void setEnvironmentIntensity(float intensity);
    float getEnvironmentIntensity() const { return environmentIntensity_; }
    
    void setFogEnabled(bool enabled);
    bool isFogEnabled() const { return fogEnabled_; }
    
    void setFogColor(const glm::vec3& color);
    glm::vec3 getFogColor() const { return fogColor_; }
    
    void setFogDensity(float density);
    float getFogDensity() const { return fogDensity_; }
    
    void setFogStart(float start);
    float getFogStart() const { return fogStart_; }
    
    void setFogEnd(float end);
    float getFogEnd() const { return fogEnd_; }
    
    void bakeLightmaps(int resolution = 1024);
    void bakeProbes();
    
    void clear();
    
    struct LightingStats {
        int totalLights;
        int shadowCastingLights;
        int lightProbes;
        float totalMemoryMB;
    };
    
    LightingStats getStats() const;

private:
    LightingSystem();
    ~LightingSystem();
    
    std::string generateId() const;
    
    std::unordered_map<std::string, std::shared_ptr<Light>> lights_;
    std::unordered_map<std::string, std::shared_ptr<LightProbe>> lightProbes_;
    
    glm::vec3 ambientColor_;
    float ambientIntensity_;
    
    std::string environmentMap_;
    float environmentIntensity_;
    
    bool fogEnabled_;
    glm::vec3 fogColor_;
    float fogDensity_;
    float fogStart_;
    float fogEnd_;
};

}
}
