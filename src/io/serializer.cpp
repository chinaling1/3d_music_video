#include "serializer.h"
#include "file_loader.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <zlib.h>

namespace v3d {
namespace io {

std::function<void(float)> Serializer::progressCallback_;
std::function<void(const std::string&)> Serializer::errorCallback_;

std::string Serializer::serializeToJson(const nlohmann::json& data) {
    try {
        std::string json = data.dump(2);
        if (progressCallback_) {
            progressCallback_(1.0f);
        }
        return json;
    } catch (const std::exception& e) {
        if (errorCallback_) {
            errorCallback_("JSON serialization error: " + std::string(e.what()));
        }
        return "{}";
    }
}

nlohmann::json Serializer::deserializeFromJson(const std::string& json) {
    try {
        nlohmann::json data = nlohmann::json::parse(json);
        if (progressCallback_) {
            progressCallback_(1.0f);
        }
        return data;
    } catch (const std::exception& e) {
        if (errorCallback_) {
            errorCallback_("JSON deserialization error: " + std::string(e.what()));
        }
        return nlohmann::json();
    }
}

std::vector<uint8_t> Serializer::serializeToBinary(const nlohmann::json& data) {
    std::string json = serializeToJson(data);
    std::vector<uint8_t> binary(json.begin(), json.end());
    return binary;
}

nlohmann::json Serializer::deserializeFromBinary(const std::vector<uint8_t>& data) {
    std::string json(data.begin(), data.end());
    return deserializeFromJson(json);
}

std::string Serializer::compress(const std::string& data) {
    uLongf sourceSize = static_cast<uLongf>(data.size());
    uLongf compressedSize = compressBound(sourceSize);

    std::vector<uint8_t> compressed(compressedSize);
    int result = compress2(compressed.data(), &compressedSize,
                          reinterpret_cast<const Bytef*>(data.c_str()), sourceSize, Z_BEST_COMPRESSION);
    
    if (result != Z_OK) {
        throw std::runtime_error("Compression failed");
    }

    return std::string(compressed.begin(), compressed.begin() + compressedSize);
}

std::string Serializer::decompress(const std::string& compressed) {
    uLongf sourceSize = static_cast<uLongf>(compressed.size());
    uLongf decompressedSize = sourceSize * 10;

    std::vector<uint8_t> decompressed(decompressedSize);
    int result = uncompress(decompressed.data(), &decompressedSize,
                           reinterpret_cast<const Bytef*>(compressed.c_str()), sourceSize);
    
    if (result != Z_OK) {
        throw std::runtime_error("Decompression failed");
    }

    return std::string(decompressed.begin(), decompressed.begin() + decompressedSize);
}

std::vector<uint8_t> Serializer::compressBinary(const std::vector<uint8_t>& data) {
    uLongf sourceSize = static_cast<uLongf>(data.size());
    uLongf compressedSize = compressBound(sourceSize);

    std::vector<uint8_t> compressed(compressedSize);
    int result = compress2(compressed.data(), &compressedSize,
                          data.data(), sourceSize, Z_BEST_COMPRESSION);
    
    if (result != Z_OK) {
        throw std::runtime_error("Compression failed");
    }

    compressed.resize(compressedSize);
    return compressed;
}

std::vector<uint8_t> Serializer::decompressBinary(const std::vector<uint8_t>& compressed) {
    uLongf sourceSize = compressed.size();
    uLongf decompressedSize = sourceSize * 10;

    std::vector<uint8_t> decompressed(decompressedSize);
    uncompress(decompressed.data(), &decompressedSize,
             compressed.data(), sourceSize);

    decompressed.resize(decompressedSize);
    return decompressed;
}

std::string Serializer::encodeBase64(const std::vector<uint8_t>& data) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    result.reserve(((data.size() + 2) / 3) * 4);

    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t value = data[i];
        value <<= 8;
        if (i + 1 < data.size()) value |= data[i + 1];
        value <<= 8;
        if (i + 2 < data.size()) value |= data[i + 2];

        result.push_back(chars[(value >> 18) & 0x3F]);
        result.push_back(chars[(value >> 12) & 0x3F]);
        result.push_back(chars[(value >> 6) & 0x3F]);
        result.push_back(chars[value & 0x3F]);
    }

    while (result.size() % 4) {
        result.push_back('=');
    }

    return result;
}

std::vector<uint8_t> Serializer::decodeBase64(const std::string& encoded) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::vector<uint8_t> result;
    result.reserve((encoded.size() / 4) * 3);

    uint32_t value = 0;
    int bits = 0;

    for (char c : encoded) {
        if (c == '=') {
            break;
        }

        int index = chars.find(c);
        if (index == std::string::npos) {
            continue;
        }

        value = (value << 6) | index;
        bits += 6;

        if (bits >= 8) {
            result.push_back((value >> (bits - 8)) & 0xFF);
            bits -= 8;
        }
    }

    return result;
}

std::string Serializer::calculateChecksum(const std::string& data) {
    uint32_t crc = 0xFFFFFFFF;

    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; ++i) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = crc >> 1;
            }
        }
    }

    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << (~crc);
    return ss.str();
}

std::string Serializer::calculateChecksum(const std::vector<uint8_t>& data) {
    return calculateChecksum(std::string(data.begin(), data.end()));
}

void Serializer::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void Serializer::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

std::string VersionedSerializer::Version::toString() const {
    std::stringstream ss;
    ss << major << "." << minor << "." << patch;
    return ss.str();
}

bool VersionedSerializer::Version::operator<(const Version& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    return patch < other.patch;
}

bool VersionedSerializer::Version::operator==(const Version& other) const {
    return major == other.major && minor == other.minor && patch == other.patch;
}

VersionedSerializer::VersionedSerializer()
    : currentVersion_{1, 0, 0}
    , minimumCompatibleVersion_{1, 0, 0} {
}

VersionedSerializer::~VersionedSerializer() {
}

void VersionedSerializer::setCurrentVersion(const Version& version) {
    currentVersion_ = version;
}

VersionedSerializer::Version VersionedSerializer::getCurrentVersion() const {
    return currentVersion_;
}

void VersionedSerializer::setMinimumCompatibleVersion(const Version& version) {
    minimumCompatibleVersion_ = version;
}

VersionedSerializer::Version VersionedSerializer::getMinimumCompatibleVersion() const {
    return minimumCompatibleVersion_;
}

template<typename T>
std::string VersionedSerializer::serialize(const T& object) {
    nlohmann::json data;

    data["version"] = {
        {"major", currentVersion_.major},
        {"minor", currentVersion_.minor},
        {"patch", currentVersion_.patch}
    };

    data["data"] = object;

    if (Serializer::progressCallback_) {
        Serializer::progressCallback_(0.5f);
    }

    std::string json = Serializer::serializeToJson(data);

    if (Serializer::progressCallback_) {
        Serializer::progressCallback_(1.0f);
    }

    return json;
}

template<typename T>
bool VersionedSerializer::deserialize(const std::string& data, T& object) {
    nlohmann::json json = Serializer::deserializeFromJson(data);

    if (!json.contains("version")) {
        if (Serializer::errorCallback_) {
            Serializer::errorCallback_("Missing version information");
        }
        return false;
    }

    auto versionJson = json["version"];
    Version version;
    version.major = versionJson.value("major", 1);
    version.minor = versionJson.value("minor", 0);
    version.patch = versionJson.value("patch", 0);

    if (!isCompatible(version)) {
        if (migrationCallback_) {
            nlohmann::json migratedData;
            if (migrationCallback_(version, currentVersion_, json["data"])) {
                object = json["data"].get<T>();
            } else {
                if (Serializer::errorCallback_) {
                    Serializer::errorCallback_("Failed to migrate data from version " + version.toString());
                }
                return false;
            }
        } else {
            if (Serializer::errorCallback_) {
                Serializer::errorCallback_("Incompatible version: " + version.toString());
            }
            return false;
        }
    } else {
        object = json["data"].get<T>();
    }

    if (Serializer::progressCallback_) {
        Serializer::progressCallback_(1.0f);
    }

    return true;
}

template<typename T>
std::vector<uint8_t> VersionedSerializer::serializeBinary(const T& object) {
    std::string json = serialize(object);
    std::vector<uint8_t> binary(json.begin(), json.end());
    return Serializer::compressBinary(binary);
}

template<typename T>
bool VersionedSerializer::deserializeBinary(const std::vector<uint8_t>& data, T& object) {
    std::vector<uint8_t> decompressed = Serializer::decompressBinary(data);
    std::string json(decompressed.begin(), decompressed.end());
    return deserialize(json, object);
}

bool VersionedSerializer::isCompatible(const Version& version) const {
    return version >= minimumCompatibleVersion_;
}

void VersionedSerializer::setMigrationCallback(std::function<bool(const Version&, const Version&, nlohmann::json&)> callback) {
    migrationCallback_ = callback;
}

ProjectSerializer::ProjectSerializer() {
}

ProjectSerializer::~ProjectSerializer() {
}

bool ProjectSerializer::saveProject(const std::string& filePath, const ProjectMetadata& metadata,
                               const nlohmann::json& projectData) {
    nlohmann::json root;

    root["metadata"] = {
        {"name", metadata.name},
        {"description", metadata.description},
        {"author", metadata.author},
        {"version", {
            {"major", metadata.version.major},
            {"minor", metadata.version.minor},
            {"patch", metadata.version.patch}
        }},
        {"createdDate", metadata.createdDate},
        {"modifiedDate", metadata.modifiedDate},
        {"thumbnail", metadata.thumbnail},
        {"customProperties", metadata.customProperties}
    };

    root["project"] = projectData;

    std::string json = Serializer::serializeToJson(root);

    if (!FileLoader::writeTextFile(filePath, json)) {
        return false;
    }

    addRecentProject(filePath);

    return true;
}

bool ProjectSerializer::loadProject(const std::string& filePath, ProjectMetadata& metadata,
                               nlohmann::json& projectData) {
    std::string json = FileLoader::readTextFile(filePath);

    nlohmann::json root = Serializer::deserializeFromJson(json);

    if (!root.contains("metadata") || !root.contains("project")) {
        if (errorCallback_) {
            errorCallback_("Invalid project file format");
        }
        return false;
    }

    auto metaJson = root["metadata"];
    metadata.name = metaJson.value("name", "");
    metadata.description = metaJson.value("description", "");
    metadata.author = metaJson.value("author", "");

    auto versionJson = metaJson["version"];
    metadata.version.major = versionJson.value("major", 1);
    metadata.version.minor = versionJson.value("minor", 0);
    metadata.version.patch = versionJson.value("patch", 0);

    metadata.createdDate = metaJson.value("createdDate", "");
    metadata.modifiedDate = metaJson.value("modifiedDate", "");
    metadata.thumbnail = metaJson.value("thumbnail", "");
    metadata.customProperties = metaJson.value("customProperties", nlohmann::json::object());

    projectData = root["project"];

    addRecentProject(filePath);

    return true;
}

bool ProjectSerializer::saveScene(const std::string& filePath, const nlohmann::json& sceneData) {
    std::string json = Serializer::serializeToJson(sceneData);
    return FileLoader::writeTextFile(filePath, json);
}

bool ProjectSerializer::loadScene(const std::string& filePath, nlohmann::json& sceneData) {
    std::string json = FileLoader::readTextFile(filePath);
    sceneData = Serializer::deserializeFromJson(json);
    return true;
}

bool ProjectSerializer::saveAsset(const std::string& filePath, const std::string& assetType,
                                const nlohmann::json& assetData) {
    nlohmann::json root;
    root["assetType"] = assetType;
    root["data"] = assetData;

    std::string json = Serializer::serializeToJson(root);
    return FileLoader::writeTextFile(filePath, json);
}

bool ProjectSerializer::loadAsset(const std::string& filePath, std::string& assetType,
                                nlohmann::json& assetData) {
    std::string json = FileLoader::readTextFile(filePath);
    nlohmann::json root = Serializer::deserializeFromJson(json);

    if (root.value("assetType", "") != assetType) {
        return false;
    }

    assetData = root["data"];
    return true;
}

std::vector<std::string> ProjectSerializer::getRecentProjects() const {
    std::string recentPath = getRecentProjectsPath();
    std::string json = FileLoader::readTextFile(recentPath);

    if (json.empty()) {
        return {};
    }

    try {
        nlohmann::json root = nlohmann::json::parse(json);
        return root.value("projects", std::vector<std::string>{});
    } catch (...) {
        return {};
    }
}

void ProjectSerializer::addRecentProject(const std::string& filePath) {
    auto projects = getRecentProjects();

    auto it = std::find(projects.begin(), projects.end(), filePath);
    if (it != projects.end()) {
        projects.erase(it);
    }

    projects.insert(projects.begin(), filePath);

    if (projects.size() > 10) {
        projects.resize(10);
    }

    nlohmann::json root;
    root["projects"] = projects;

    std::string recentPath = getRecentProjectsPath();
    FileLoader::writeTextFile(recentPath, root.dump(2));
}

void ProjectSerializer::setProgressCallback(std::function<void(float)> callback) {
    Serializer::setProgressCallback(callback);
}

void ProjectSerializer::setErrorCallback(std::function<void(const std::string&)> callback) {
    Serializer::setErrorCallback(callback);
}

std::string ProjectSerializer::getRecentProjectsPath() const {
    std::string appDataPath;
    const char* envPath = std::getenv("APPDATA");
    if (envPath && *envPath) {
        appDataPath = envPath;
    } else {
        appDataPath = ".";
    }
    std::string studioPath = FileLoader::joinPath(appDataPath, ".3dstudio");
    FileLoader::createDirectory(studioPath);
    return FileLoader::joinPath(studioPath, "recent_projects.json");
}

void ProjectSerializer::saveRecentProjects() const {
}

}
}