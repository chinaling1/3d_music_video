#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace v3d {
namespace io {

class Serializer {
public:
    static std::string serializeToJson(const nlohmann::json& data);
    static nlohmann::json deserializeFromJson(const std::string& json);

    static std::vector<uint8_t> serializeToBinary(const nlohmann::json& data);
    static nlohmann::json deserializeFromBinary(const std::vector<uint8_t>& data);

    static std::string compress(const std::string& data);
    static std::string decompress(const std::string& compressed);

    static std::vector<uint8_t> compressBinary(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> decompressBinary(const std::vector<uint8_t>& compressed);

    static std::string encodeBase64(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> decodeBase64(const std::string& encoded);

    static std::string calculateChecksum(const std::string& data);
    static std::string calculateChecksum(const std::vector<uint8_t>& data);

    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

class VersionedSerializer {
public:
    struct Version {
        int major;
        int minor;
        int patch;

        std::string toString() const;
        bool operator<(const Version& other) const;
        bool operator==(const Version& other) const;
        bool operator>=(const Version& other) const { return !(*this < other) || *this == other; }
    };

    VersionedSerializer();
    ~VersionedSerializer();

    void setCurrentVersion(const Version& version);
    Version getCurrentVersion() const;

    void setMinimumCompatibleVersion(const Version& version);
    Version getMinimumCompatibleVersion() const;

    template<typename T>
    std::string serialize(const T& object);

    template<typename T>
    bool deserialize(const std::string& data, T& object);

    template<typename T>
    std::vector<uint8_t> serializeBinary(const T& object);

    template<typename T>
    bool deserializeBinary(const std::vector<uint8_t>& data, T& object);

    bool isCompatible(const Version& version) const;

    void setMigrationCallback(std::function<bool(const Version&, const Version&, nlohmann::json&)> callback);

private:
    Version currentVersion_;
    Version minimumCompatibleVersion_;
    std::function<bool(const Version&, const Version&, nlohmann::json&)> migrationCallback_;
};

class ProjectSerializer {
public:
    struct ProjectMetadata {
        std::string name;
        std::string description;
        std::string author;
        VersionedSerializer::Version version;
        std::string createdDate;
        std::string modifiedDate;
        std::string thumbnail;
        std::unordered_map<std::string, std::string> customProperties;
    };

    ProjectSerializer();
    ~ProjectSerializer();

    bool saveProject(const std::string& filePath, const ProjectMetadata& metadata,
                     const nlohmann::json& projectData);

    bool loadProject(const std::string& filePath, ProjectMetadata& metadata,
                     nlohmann::json& projectData);

    bool saveScene(const std::string& filePath, const nlohmann::json& sceneData);
    bool loadScene(const std::string& filePath, nlohmann::json& sceneData);

    bool saveAsset(const std::string& filePath, const std::string& assetType,
                  const nlohmann::json& assetData);
    bool loadAsset(const std::string& filePath, std::string& assetType,
                  nlohmann::json& assetData);

    std::vector<std::string> getRecentProjects() const;
    void addRecentProject(const std::string& filePath);

    void setProgressCallback(std::function<void(float)> callback);
    void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    std::string getRecentProjectsPath() const;
    void saveRecentProjects() const;

    std::function<void(float)> progressCallback_;
    std::function<void(const std::string&)> errorCallback_;
};

}
}