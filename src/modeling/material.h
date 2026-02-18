#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

namespace v3d {
namespace modeling {

enum class MaterialType {
    Standard,
    Unlit,
    Normal,
    Physical,
    Toon,
    Glass,
    Water,
    Skybox
};

struct TextureSlot {
    std::string path;
    int textureId;
    glm::vec2 tiling{1.0f};
    glm::vec2 offset{0.0f};
    bool enabled;

    TextureSlot() : textureId(-1), enabled(false) {}
};

class Material {
public:
    Material();
    explicit Material(const std::string& name);
    ~Material();

    std::string getName() const;
    void setName(const std::string& name);

    MaterialType getType() const;
    void setType(MaterialType type);

    glm::vec3 getAlbedo() const;
    void setAlbedo(const glm::vec3& albedo);

    float getMetallic() const;
    void setMetallic(float metallic);

    float getRoughness() const;
    void setRoughness(float roughness);

    float getAo() const;
    void setAo(float ao);

    glm::vec3 getEmissive() const;
    void setEmissive(const glm::vec3& emissive);

    float getEmissiveIntensity() const;
    void setEmissiveIntensity(float intensity);

    float getNormalScale() const;
    void setNormalScale(float scale);

    float getOpacity() const;
    void setOpacity(float opacity);

    float getIOR() const;
    void setIOR(float ior);

    float getTransmission() const;
    void setTransmission(float transmission);

    float getThickness() const;
    void setThickness(float thickness);

    bool isDoubleSided() const;
    void setDoubleSided(bool doubleSided);

    const TextureSlot& getAlbedoTexture() const;
    void setAlbedoTexture(const std::string& path);

    const TextureSlot& getNormalTexture() const;
    void setNormalTexture(const std::string& path);

    const TextureSlot& getMetallicTexture() const;
    void setMetallicTexture(const std::string& path);

    const TextureSlot& getRoughnessTexture() const;
    void setRoughnessTexture(const std::string& path);

    const TextureSlot& getAoTexture() const;
    void setAoTexture(const std::string& path);

    const TextureSlot& getEmissiveTexture() const;
    void setEmissiveTexture(const std::string& path);

    const TextureSlot& getOpacityTexture() const;
    void setOpacityTexture(const std::string& path);

    void setTextureTiling(const std::string& slot, const glm::vec2& tiling);
    void setTextureOffset(const std::string& slot, const glm::vec2& offset);

    void enableTexture(const std::string& slot);
    void disableTexture(const std::string& slot);

    bool isTransparent() const;

    void clear();

private:
    std::string name_;
    MaterialType type_;

    glm::vec3 albedo_;
    float metallic_;
    float roughness_;
    float ao_;
    glm::vec3 emissive_;
    float emissiveIntensity_;
    float normalScale_;
    float opacity_;
    float ior_;
    float transmission_;
    float thickness_;
    bool doubleSided_;

    TextureSlot albedoTexture_;
    TextureSlot normalTexture_;
    TextureSlot metallicTexture_;
    TextureSlot roughnessTexture_;
    TextureSlot aoTexture_;
    TextureSlot emissiveTexture_;
    TextureSlot opacityTexture_;
};

class MaterialLibrary {
public:
    static MaterialLibrary& getInstance();

    std::shared_ptr<Material> createMaterial(const std::string& name);
    std::shared_ptr<Material> getMaterial(const std::string& name) const;

    void addMaterial(std::shared_ptr<Material> material);
    void removeMaterial(const std::string& name);
    void clear();

    std::vector<std::string> getMaterialNames() const;
    size_t getMaterialCount() const;

    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath) const;

    std::shared_ptr<Material> getDefaultMaterial() const;
    void setDefaultMaterial(std::shared_ptr<Material> material);

private:
    MaterialLibrary();

    std::unordered_map<std::string, std::shared_ptr<Material>> materials_;
    std::shared_ptr<Material> defaultMaterial_;
};

}
}