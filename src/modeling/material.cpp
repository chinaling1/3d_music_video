#include "material.h"
#include <algorithm>
#include <glm/glm.hpp>

namespace v3d {
namespace modeling {

Material::Material()
    : type_(MaterialType::Standard)
    , albedo_(1.0f)
    , metallic_(0.0f)
    , roughness_(0.5f)
    , ao_(1.0f)
    , emissive_(0.0f)
    , emissiveIntensity_(1.0f)
    , normalScale_(1.0f)
    , opacity_(1.0f)
    , ior_(1.5f)
    , transmission_(0.0f)
    , thickness_(0.5f)
    , doubleSided_(false) {
}

Material::Material(const std::string& name)
    : name_(name)
    , type_(MaterialType::Standard)
    , albedo_(1.0f)
    , metallic_(0.0f)
    , roughness_(0.5f)
    , ao_(1.0f)
    , emissive_(0.0f)
    , emissiveIntensity_(1.0f)
    , normalScale_(1.0f)
    , opacity_(1.0f)
    , ior_(1.5f)
    , transmission_(0.0f)
    , thickness_(0.5f)
    , doubleSided_(false) {
}

Material::~Material() {
}

std::string Material::getName() const {
    return name_;
}

void Material::setName(const std::string& name) {
    name_ = name;
}

MaterialType Material::getType() const {
    return type_;
}

void Material::setType(MaterialType type) {
    type_ = type;
}

glm::vec3 Material::getAlbedo() const {
    return albedo_;
}

void Material::setAlbedo(const glm::vec3& albedo) {
    albedo_ = glm::clamp(albedo, 0.0f, 1.0f);
}

float Material::getMetallic() const {
    return metallic_;
}

void Material::setMetallic(float metallic) {
    metallic_ = glm::clamp(metallic, 0.0f, 1.0f);
}

float Material::getRoughness() const {
    return roughness_;
}

void Material::setRoughness(float roughness) {
    roughness_ = glm::clamp(roughness, 0.0f, 1.0f);
}

float Material::getAo() const {
    return ao_;
}

void Material::setAo(float ao) {
    ao_ = glm::clamp(ao, 0.0f, 1.0f);
}

glm::vec3 Material::getEmissive() const {
    return emissive_;
}

void Material::setEmissive(const glm::vec3& emissive) {
    emissive_ = glm::clamp(emissive, 0.0f, 1.0f);
}

float Material::getEmissiveIntensity() const {
    return emissiveIntensity_;
}

void Material::setEmissiveIntensity(float intensity) {
    emissiveIntensity_ = std::max(0.0f, intensity);
}

float Material::getNormalScale() const {
    return normalScale_;
}

void Material::setNormalScale(float scale) {
    normalScale_ = std::max(0.0f, scale);
}

float Material::getOpacity() const {
    return opacity_;
}

void Material::setOpacity(float opacity) {
    opacity_ = glm::clamp(opacity, 0.0f, 1.0f);
}

float Material::getIOR() const {
    return ior_;
}

void Material::setIOR(float ior) {
    ior_ = std::max(1.0f, ior);
}

float Material::getTransmission() const {
    return transmission_;
}

void Material::setTransmission(float transmission) {
    transmission_ = glm::clamp(transmission, 0.0f, 1.0f);
}

float Material::getThickness() const {
    return thickness_;
}

void Material::setThickness(float thickness) {
    thickness_ = std::max(0.0f, thickness);
}

bool Material::isDoubleSided() const {
    return doubleSided_;
}

void Material::setDoubleSided(bool doubleSided) {
    doubleSided_ = doubleSided;
}

const TextureSlot& Material::getAlbedoTexture() const {
    return albedoTexture_;
}

void Material::setAlbedoTexture(const std::string& path) {
    albedoTexture_.path = path;
    albedoTexture_.enabled = !path.empty();
}

const TextureSlot& Material::getNormalTexture() const {
    return normalTexture_;
}

void Material::setNormalTexture(const std::string& path) {
    normalTexture_.path = path;
    normalTexture_.enabled = !path.empty();
}

const TextureSlot& Material::getMetallicTexture() const {
    return metallicTexture_;
}

void Material::setMetallicTexture(const std::string& path) {
    metallicTexture_.path = path;
    metallicTexture_.enabled = !path.empty();
}

const TextureSlot& Material::getRoughnessTexture() const {
    return roughnessTexture_;
}

void Material::setRoughnessTexture(const std::string& path) {
    roughnessTexture_.path = path;
    roughnessTexture_.enabled = !path.empty();
}

const TextureSlot& Material::getAoTexture() const {
    return aoTexture_;
}

void Material::setAoTexture(const std::string& path) {
    aoTexture_.path = path;
    aoTexture_.enabled = !path.empty();
}

const TextureSlot& Material::getEmissiveTexture() const {
    return emissiveTexture_;
}

void Material::setEmissiveTexture(const std::string& path) {
    emissiveTexture_.path = path;
    emissiveTexture_.enabled = !path.empty();
}

const TextureSlot& Material::getOpacityTexture() const {
    return opacityTexture_;
}

void Material::setOpacityTexture(const std::string& path) {
    opacityTexture_.path = path;
    opacityTexture_.enabled = !path.empty();
}

void Material::setTextureTiling(const std::string& slot, const glm::vec2& tiling) {
    if (slot == "albedo") {
        albedoTexture_.tiling = tiling;
    } else if (slot == "normal") {
        normalTexture_.tiling = tiling;
    } else if (slot == "metallic") {
        metallicTexture_.tiling = tiling;
    } else if (slot == "roughness") {
        roughnessTexture_.tiling = tiling;
    } else if (slot == "ao") {
        aoTexture_.tiling = tiling;
    } else if (slot == "emissive") {
        emissiveTexture_.tiling = tiling;
    } else if (slot == "opacity") {
        opacityTexture_.tiling = tiling;
    }
}

void Material::setTextureOffset(const std::string& slot, const glm::vec2& offset) {
    if (slot == "albedo") {
        albedoTexture_.offset = offset;
    } else if (slot == "normal") {
        normalTexture_.offset = offset;
    } else if (slot == "metallic") {
        metallicTexture_.offset = offset;
    } else if (slot == "roughness") {
        roughnessTexture_.offset = offset;
    } else if (slot == "ao") {
        aoTexture_.offset = offset;
    } else if (slot == "emissive") {
        emissiveTexture_.offset = offset;
    } else if (slot == "opacity") {
        opacityTexture_.offset = offset;
    }
}

void Material::enableTexture(const std::string& slot) {
    if (slot == "albedo") {
        albedoTexture_.enabled = true;
    } else if (slot == "normal") {
        normalTexture_.enabled = true;
    } else if (slot == "metallic") {
        metallicTexture_.enabled = true;
    } else if (slot == "roughness") {
        roughnessTexture_.enabled = true;
    } else if (slot == "ao") {
        aoTexture_.enabled = true;
    } else if (slot == "emissive") {
        emissiveTexture_.enabled = true;
    } else if (slot == "opacity") {
        opacityTexture_.enabled = true;
    }
}

void Material::disableTexture(const std::string& slot) {
    if (slot == "albedo") {
        albedoTexture_.enabled = false;
    } else if (slot == "normal") {
        normalTexture_.enabled = false;
    } else if (slot == "metallic") {
        metallicTexture_.enabled = false;
    } else if (slot == "roughness") {
        roughnessTexture_.enabled = false;
    } else if (slot == "ao") {
        aoTexture_.enabled = false;
    } else if (slot == "emissive") {
        emissiveTexture_.enabled = false;
    } else if (slot == "opacity") {
        opacityTexture_.enabled = false;
    }
}

bool Material::isTransparent() const {
    return opacity_ < 1.0f || opacityTexture_.enabled;
}

void Material::clear() {
    name_.clear();
    type_ = MaterialType::Standard;
    albedo_ = glm::vec3(1.0f);
    metallic_ = 0.0f;
    roughness_ = 0.5f;
    ao_ = 1.0f;
    emissive_ = glm::vec3(0.0f);
    emissiveIntensity_ = 1.0f;
    normalScale_ = 1.0f;
    opacity_ = 1.0f;
    ior_ = 1.5f;
    transmission_ = 0.0f;
    thickness_ = 0.5f;
    doubleSided_ = false;

    albedoTexture_ = TextureSlot();
    normalTexture_ = TextureSlot();
    metallicTexture_ = TextureSlot();
    roughnessTexture_ = TextureSlot();
    aoTexture_ = TextureSlot();
    emissiveTexture_ = TextureSlot();
    opacityTexture_ = TextureSlot();
}

MaterialLibrary& MaterialLibrary::getInstance() {
    static MaterialLibrary instance;
    return instance;
}

MaterialLibrary::MaterialLibrary() {
    defaultMaterial_ = std::make_shared<Material>("Default");
}

std::shared_ptr<Material> MaterialLibrary::createMaterial(const std::string& name) {
    auto material = std::make_shared<Material>(name);
    materials_[name] = material;
    return material;
}

std::shared_ptr<Material> MaterialLibrary::getMaterial(const std::string& name) const {
    auto it = materials_.find(name);
    return it != materials_.end() ? it->second : defaultMaterial_;
}

void MaterialLibrary::addMaterial(std::shared_ptr<Material> material) {
    if (material) {
        materials_[material->getName()] = material;
    }
}

void MaterialLibrary::removeMaterial(const std::string& name) {
    materials_.erase(name);
}

void MaterialLibrary::clear() {
    materials_.clear();
}

std::vector<std::string> MaterialLibrary::getMaterialNames() const {
    std::vector<std::string> names;
    for (const auto& pair : materials_) {
        names.push_back(pair.first);
    }
    return names;
}

size_t MaterialLibrary::getMaterialCount() const {
    return materials_.size();
}

bool MaterialLibrary::loadFromFile(const std::string& filePath) {
    return false;
}

bool MaterialLibrary::saveToFile(const std::string& filePath) const {
    return false;
}

std::shared_ptr<Material> MaterialLibrary::getDefaultMaterial() const {
    return defaultMaterial_;
}

void MaterialLibrary::setDefaultMaterial(std::shared_ptr<Material> material) {
    defaultMaterial_ = material;
}

}
}