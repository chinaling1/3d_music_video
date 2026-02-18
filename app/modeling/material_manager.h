/**
 * @file material_manager.h
 * @brief 材质纹理管理器 - 管理材质和纹理资源
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

namespace v3d {
namespace modeling {

enum class TextureType {
    Diffuse,
    Specular,
    Normal,
    Bump,
    Roughness,
    Metallic,
    AmbientOcclusion,
    Emissive,
    Height,
    Opacity
};

struct Texture {
    std::string id;
    std::string name;
    std::string filePath;
    TextureType type;
    int width;
    int height;
    int channels;
    std::vector<uint8_t> data;
    bool isLoaded;
    bool isEmbedded;
    
    Texture() : type(TextureType::Diffuse), width(0), height(0), channels(0), isLoaded(false), isEmbedded(false) {}
};

struct TextureSlot {
    std::shared_ptr<Texture> texture;
    bool enabled;
    float blendFactor;
    glm::vec2 offset;
    glm::vec2 scale;
    float rotation;
    std::string uvMap;
    
    TextureSlot() : enabled(true), blendFactor(1.0f), offset(0), scale(1), rotation(0) {}
};

enum class MaterialType {
    Standard,
    PBR,
    Unlit,
    Toon,
    Custom
};

enum class BlendMode {
    Opaque,
    AlphaBlend,
    AlphaClip,
    Additive
};

struct Material {
    std::string id;
    std::string name;
    MaterialType type;
    BlendMode blendMode;
    
    glm::vec4 baseColor;
    glm::vec3 emissiveColor;
    float emissiveStrength;
    
    float metallic;
    float roughness;
    float specular;
    float specularTint;
    
    float clearcoat;
    float clearcoatRoughness;
    
    float sheen;
    glm::vec3 sheenTint;
    
    float anisotropic;
    float anisotropicRotation;
    
    float ior;
    float transmission;
    float transmissionRoughness;
    
    float alpha;
    float alphaCutoff;
    
    glm::vec2 subsurfaceRadius;
    float subsurfaceWeight;
    glm::vec3 subsurfaceColor;
    
    std::unordered_map<TextureType, TextureSlot> textures;
    
    int shaderId;
    bool doubleSided;
    bool shadowCasting;
    bool shadowReceiving;
    
    Material();
    
    void setTexture(TextureType type, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(TextureType type) const;
    bool hasTexture(TextureType type) const;
    void removeTexture(TextureType type);
    
    void loadFromFile(const std::string& filePath);
    void saveToFile(const std::string& filePath) const;
};

class MaterialManager {
public:
    static MaterialManager& getInstance();
    
    std::shared_ptr<Material> createMaterial(const std::string& name);
    std::shared_ptr<Material> getMaterial(const std::string& id) const;
    std::shared_ptr<Material> getMaterialByName(const std::string& name) const;
    void removeMaterial(const std::string& id);
    void renameMaterial(const std::string& id, const std::string& newName);
    
    std::shared_ptr<Texture> loadTexture(const std::string& filePath, TextureType type = TextureType::Diffuse);
    std::shared_ptr<Texture> createTexture(const std::string& name, int width, int height, int channels, const uint8_t* data);
    std::shared_ptr<Texture> getTexture(const std::string& id) const;
    void removeTexture(const std::string& id);
    
    std::vector<std::shared_ptr<Material>> getAllMaterials() const;
    std::vector<std::shared_ptr<Texture>> getAllTextures() const;
    
    std::shared_ptr<Material> getDefaultMaterial();
    std::shared_ptr<Material> cloneMaterial(const std::string& id, const std::string& newName);
    
    void setDefaultMaterial(std::shared_ptr<Material> material);
    
    void clear();
    
    void exportMaterialLibrary(const std::string& filePath);
    void importMaterialLibrary(const std::string& filePath);

private:
    MaterialManager();
    ~MaterialManager();
    
    std::string generateId() const;
    
    std::unordered_map<std::string, std::shared_ptr<Material>> materials_;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
    std::shared_ptr<Material> defaultMaterial_;
};

}
}
