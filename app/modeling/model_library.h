/**
 * @file model_library.h
 * @brief 模型库系统 - 管理3D模型资源
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "geometry_factory.h"

namespace v3d {
namespace modeling {

enum class ModelCategory {
    Primitive,
    Furniture,
    Character,
    Vehicle,
    Architecture,
    Nature,
    Electronics,
    Food,
    Animal,
    Weapon,
    Tool,
    Decoration,
    Industrial,
    Medical,
    Sports,
    Musical,
    Clothing,
    Accessory,
    Custom
};

struct ModelThumbnail {
    int width;
    int height;
    std::vector<uint8_t> data;
    
    ModelThumbnail() : width(0), height(0) {}
};

struct ModelMetadata {
    std::string id;
    std::string name;
    std::string description;
    ModelCategory category;
    std::vector<std::string> tags;
    std::string author;
    std::string license;
    std::string version;
    
    int vertexCount;
    int faceCount;
    int materialCount;
    int textureCount;
    
    glm::vec3 boundingMin;
    glm::vec3 boundingMax;
    glm::vec3 center;
    float radius;
    
    std::string filePath;
    std::string thumbnailPath;
    ModelThumbnail thumbnail;
    
    std::chrono::system_clock::time_point createdTime;
    std::chrono::system_clock::time_point modifiedTime;
    
    bool isBuiltIn;
    bool isFavorite;
    float rating;
    int downloadCount;
    
    ModelMetadata()
        : category(ModelCategory::Primitive), vertexCount(0), faceCount(0)
        , materialCount(0), textureCount(0), boundingMin(0), boundingMax(0)
        , center(0), radius(0), isBuiltIn(false), isFavorite(false)
        , rating(0), downloadCount(0) {}
};

struct ModelResource {
    ModelMetadata metadata;
    MeshData mesh;
    std::vector<std::shared_ptr<class Material>> materials;
    std::vector<std::shared_ptr<class Texture>> textures;
    bool isLoaded;
    
    ModelResource() : isLoaded(false) {}
};

class ModelLibrary {
public:
    using SearchCallback = std::function<void(const std::vector<ModelMetadata>&)>;
    using LoadCallback = std::function<void(std::shared_ptr<ModelResource>)>;
    
    static ModelLibrary& getInstance();
    
    void initialize();
    void shutdown();
    
    std::shared_ptr<ModelResource> getModel(const std::string& id);
    std::shared_ptr<ModelResource> loadModel(const std::string& filePath);
    void unloadModel(const std::string& id);
    
    void importModel(const std::string& filePath, ModelCategory category = ModelCategory::Custom);
    void exportModel(const std::string& id, const std::string& filePath, const std::string& format);
    
    std::vector<ModelMetadata> search(const std::string& query);
    std::vector<ModelMetadata> getByCategory(ModelCategory category);
    std::vector<ModelMetadata> getByTag(const std::string& tag);
    std::vector<ModelMetadata> getFavorites();
    std::vector<ModelMetadata> getRecent();
    std::vector<ModelMetadata> getAll();
    
    void searchAsync(const std::string& query, SearchCallback callback);
    
    void setFavorite(const std::string& id, bool favorite);
    void setRating(const std::string& id, float rating);
    void addTag(const std::string& id, const std::string& tag);
    void removeTag(const std::string& id, const std::string& tag);
    
    void setLibraryPath(const std::string& path);
    std::string getLibraryPath() const { return libraryPath_; }
    
    void scanLibrary();
    void rebuildIndex();
    
    void generateThumbnail(const std::string& id);
    void generateAllThumbnails();
    
    void downloadOnlineModel(const std::string& modelUrl, LoadCallback callback);
    void browseOnlineLibrary();
    
    void registerBuiltInModels();
    
    std::shared_ptr<ModelResource> createPrimitive(const std::string& type, const GeometryParams& params = GeometryParams());
    
    struct LibraryStats {
        int totalModels;
        int totalSizeMB;
        int categories;
        int builtInModels;
        int customModels;
    };
    
    LibraryStats getStats() const;

private:
    ModelLibrary();
    ~ModelLibrary();
    
    std::string generateId() const;
    void loadMetadata(const std::string& filePath);
    void saveMetadata(const std::string& id);
    void updateIndex();
    
    std::unordered_map<std::string, std::shared_ptr<ModelResource>> models_;
    std::unordered_map<ModelCategory, std::vector<std::string>> categoryIndex_;
    std::unordered_map<std::string, std::vector<std::string>> tagIndex_;
    std::vector<std::string> recentModels_;
    
    std::string libraryPath_;
    bool initialized_;
};

}
}
