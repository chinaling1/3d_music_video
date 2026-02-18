#include "file_saver.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace v3d {
namespace io {

std::function<void(float)> MeshLoader::progressCallback_;
std::function<void(const std::string&)> MeshLoader::errorCallback_;

std::shared_ptr<modeling::Mesh> MeshLoader::loadOBJ(const std::string& filePath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_GenUVCoords |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        if (errorCallback_) {
            errorCallback_("Failed to load OBJ file: " + filePath);
        }
        return nullptr;
    }

    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName(FileLoader::getFileName(filePath));

    if (progressCallback_) {
        progressCallback_(0.5f);
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* aiMesh = scene->mMeshes[m];

        std::unordered_map<std::string, int> vertexMap;

        for (unsigned int v = 0; v < aiMesh->mNumVertices; ++v) {
            modeling::Vertex vertex;

            vertex.position = glm::vec3(
                aiMesh->mVertices[v].x,
                aiMesh->mVertices[v].y,
                aiMesh->mVertices[v].z
            );

            if (aiMesh->mNormals) {
                vertex.normal = glm::vec3(
                    aiMesh->mNormals[v].x,
                    aiMesh->mNormals[v].y,
                    aiMesh->mNormals[v].z
                );
            }

            if (aiMesh->mTextureCoords[0]) {
                vertex.texCoord = glm::vec2(
                    aiMesh->mTextureCoords[0][v].x,
                    aiMesh->mTextureCoords[0][v].y
                );
            }

            int vertexIndex = mesh->addVertex(vertex);
        }

        for (unsigned int f = 0; f < aiMesh->mNumFaces; ++f) {
            const aiFace& face = aiMesh->mFaces[f];

            if (face.mNumIndices == 3) {
                mesh->addTriangle(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            } else if (face.mNumIndices == 4) {
                mesh->addQuad(face.mIndices[0], face.mIndices[1], face.mIndices[2], face.mIndices[3]);
            }
        }
    }

    mesh->recalculateAll();

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return mesh;
}

std::shared_ptr<modeling::Mesh> MeshLoader::loadFBX(const std::string& filePath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_GenUVCoords |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        if (errorCallback_) {
            errorCallback_("Failed to load FBX file: " + filePath);
        }
        return nullptr;
    }

    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName(FileLoader::getFileName(filePath));

    if (progressCallback_) {
        progressCallback_(0.5f);
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* aiMesh = scene->mMeshes[m];

        for (unsigned int v = 0; v < aiMesh->mNumVertices; ++v) {
            modeling::Vertex vertex;

            vertex.position = glm::vec3(
                aiMesh->mVertices[v].x,
                aiMesh->mVertices[v].y,
                aiMesh->mVertices[v].z
            );

            if (aiMesh->mNormals) {
                vertex.normal = glm::vec3(
                    aiMesh->mNormals[v].x,
                    aiMesh->mNormals[v].y,
                    aiMesh->mNormals[v].z
                );
            }

            if (aiMesh->mTextureCoords[0]) {
                vertex.texCoord = glm::vec2(
                    aiMesh->mTextureCoords[0][v].x,
                    aiMesh->mTextureCoords[0][v].y
                );
            }

            mesh->addVertex(vertex);
        }

        for (unsigned int f = 0; f < aiMesh->mNumFaces; ++f) {
            const aiFace& face = aiMesh->mFaces[f];

            if (face.mNumIndices == 3) {
                mesh->addTriangle(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            } else if (face.mNumIndices == 4) {
                mesh->addQuad(face.mIndices[0], face.mIndices[1], face.mIndices[2], face.mIndices[3]);
            }
        }
    }

    mesh->recalculateAll();

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return mesh;
}

std::shared_ptr<modeling::Mesh> MeshLoader::loadGLTF(const std::string& filePath) {
    return loadFBX(filePath);
}

std::shared_ptr<modeling::Mesh> MeshLoader::loadGLB(const std::string& filePath) {
    return loadFBX(filePath);
}

std::shared_ptr<modeling::Mesh> MeshLoader::loadDAE(const std::string& filePath) {
    return loadFBX(filePath);
}

std::shared_ptr<modeling::Mesh> MeshLoader::loadSTL(const std::string& filePath) {
    return loadFBX(filePath);
}

std::shared_ptr<modeling::Mesh> MeshLoader::loadPLY(const std::string& filePath) {
    return loadFBX(filePath);
}

std::shared_ptr<modeling::Mesh> MeshLoader::loadPMX(const std::string& filePath) {
    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName(FileLoader::getFileName(filePath));

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to open PMX file: " + filePath);
        }
        return nullptr;
    }

    char header[4];
    file.read(header, 4);

    if (header[0] != 'P' || header[1] != 'M' || header[2] != 'X' || header[3] != ' ') {
        if (errorCallback_) {
            errorCallback_("Invalid PMX file format");
        }
        return nullptr;
    }

    float version;
    file.read(reinterpret_cast<char*>(&version), 4);

    uint8_t vertexCount;
    file.read(reinterpret_cast<char*>(&vertexCount), 1);

    std::vector<glm::vec3> positions(vertexCount);
    std::vector<glm::vec3> normals(vertexCount);
    std::vector<glm::vec2> uvs(vertexCount);

    for (int i = 0; i < vertexCount; ++i) {
        float x, y, z;
        file.read(reinterpret_cast<char*>(&x), 4);
        file.read(reinterpret_cast<char*>(&y), 4);
        file.read(reinterpret_cast<char*>(&z), 4);
        positions[i] = glm::vec3(x, y, z);

        file.read(reinterpret_cast<char*>(&x), 4);
        file.read(reinterpret_cast<char*>(&y), 4);
        file.read(reinterpret_cast<char*>(&z), 4);
        normals[i] = glm::vec3(x, y, z);

        float u, v;
        file.read(reinterpret_cast<char*>(&u), 4);
        file.read(reinterpret_cast<char*>(&v), 4);
        uvs[i] = glm::vec2(u, v);
    }

    for (int i = 0; i < vertexCount; ++i) {
        modeling::Vertex vertex;
        vertex.position = positions[i];
        vertex.normal = normals[i];
        vertex.texCoord = uvs[i];
        mesh->addVertex(vertex);
    }

    uint32_t indexCount;
    file.read(reinterpret_cast<char*>(&indexCount), 4);

    for (uint32_t i = 0; i < indexCount; i += 3) {
        uint8_t i0, i1, i2;
        file.read(reinterpret_cast<char*>(&i0), 1);
        file.read(reinterpret_cast<char*>(&i1), 1);
        file.read(reinterpret_cast<char*>(&i2), 1);
        mesh->addTriangle(i0, i1, i2);
    }

    mesh->recalculateAll();

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return mesh;
}

std::shared_ptr<modeling::Mesh> MeshLoader::load(const std::string& filePath) {
    FileFormat format = FileLoader::detectFormat(filePath);

    switch (format) {
        case FileFormat::OBJ: return loadOBJ(filePath);
        case FileFormat::FBX: return loadFBX(filePath);
        case FileFormat::GLTF: return loadGLTF(filePath);
        case FileFormat::GLB: return loadGLB(filePath);
        case FileFormat::DAE: return loadDAE(filePath);
        case FileFormat::STL: return loadSTL(filePath);
        case FileFormat::PLY: return loadPLY(filePath);
        case FileFormat::PMX: return loadPMX(filePath);
        default:
            if (errorCallback_) {
                errorCallback_("Unsupported file format: " + filePath);
            }
            return nullptr;
    }
}

bool MeshLoader::saveOBJ(const std::string& filePath, const modeling::Mesh& mesh) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create OBJ file: " + filePath);
        }
        return false;
    }

    file << "# OBJ file generated by 3D Video Studio\n";
    file << "# Vertices: " << mesh.getVertexCount() << "\n";
    file << "# Faces: " << mesh.getFaceCount() << "\n\n";

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        const modeling::Vertex& vertex = mesh.getVertex(i);
        file << "v " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << "\n";
    }

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        const modeling::Vertex& vertex = mesh.getVertex(i);
        file << "vn " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z << "\n";
    }

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        const modeling::Vertex& vertex = mesh.getVertex(i);
        file << "vt " << vertex.texCoord.x << " " << vertex.texCoord.y << "\n";
    }

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const modeling::Face& face = mesh.getFace(i);

        if (face.vertices.size() == 3) {
            file << "f " << (face.vertices[0] + 1) << "/" << (face.vertices[0] + 1) << "/" << (face.vertices[0] + 1)
                 << " " << (face.vertices[1] + 1) << "/" << (face.vertices[1] + 1) << "/" << (face.vertices[1] + 1)
                 << " " << (face.vertices[2] + 1) << "/" << (face.vertices[2] + 1) << "/" << (face.vertices[2] + 1) << "\n";
        } else if (face.vertices.size() == 4) {
            file << "f " << (face.vertices[0] + 1) << "/" << (face.vertices[0] + 1) << "/" << (face.vertices[0] + 1)
                 << " " << (face.vertices[1] + 1) << "/" << (face.vertices[1] + 1) << "/" << (face.vertices[1] + 1)
                 << " " << (face.vertices[2] + 1) << "/" << (face.vertices[2] + 1) << "/" << (face.vertices[2] + 1)
                 << " " << (face.vertices[3] + 1) << "/" << (face.vertices[3] + 1) << "/" << (face.vertices[3] + 1) << "\n";
        }
    }

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool MeshLoader::saveSTL(const std::string& filePath, const modeling::Mesh& mesh) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create STL file: " + filePath);
        }
        return false;
    }

    char header[80] = "STL file generated by 3D Video Studio";
    file.write(header, 80);

    uint32_t faceCount = mesh.getFaceCount();
    file.write(reinterpret_cast<const char*>(&faceCount), 4);

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const modeling::Face& face = mesh.getFace(i);

        if (face.vertices.size() >= 3) {
            const modeling::Vertex& v0 = mesh.getVertex(face.vertices[0]);
            const modeling::Vertex& v1 = mesh.getVertex(face.vertices[1]);
            const modeling::Vertex& v2 = mesh.getVertex(face.vertices[2]);

            float normal[3] = {face.normal.x, face.normal.y, face.normal.z};
            file.write(reinterpret_cast<const char*>(normal), 12);

            file.write(reinterpret_cast<const char*>(&v0.position.x), 4);
            file.write(reinterpret_cast<const char*>(&v0.position.y), 4);
            file.write(reinterpret_cast<const char*>(&v0.position.z), 4);

            file.write(reinterpret_cast<const char*>(&v1.position.x), 4);
            file.write(reinterpret_cast<const char*>(&v1.position.y), 4);
            file.write(reinterpret_cast<const char*>(&v1.position.z), 4);

            file.write(reinterpret_cast<const char*>(&v2.position.x), 4);
            file.write(reinterpret_cast<const char*>(&v2.position.y), 4);
            file.write(reinterpret_cast<const char*>(&v2.position.z), 4);

            uint16_t attributeCount = 0;
            file.write(reinterpret_cast<const char*>(&attributeCount), 2);
        }
    }

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool MeshLoader::savePLY(const std::string& filePath, const modeling::Mesh& mesh) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create PLY file: " + filePath);
        }
        return false;
    }

    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex " << mesh.getVertexCount() << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "property float nx\n";
    file << "property float ny\n";
    file << "property float nz\n";
    file << "property float s\n";
    file << "property float t\n";
    file << "element face " << mesh.getFaceCount() << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";

    for (int i = 0; i < mesh.getVertexCount(); ++i) {
        const modeling::Vertex& vertex = mesh.getVertex(i);
        file << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << " "
             << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z << " "
             << vertex.texCoord.x << " " << vertex.texCoord.y << "\n";
    }

    for (int i = 0; i < mesh.getFaceCount(); ++i) {
        const modeling::Face& face = mesh.getFace(i);
        file << face.vertices.size();
        for (int vi : face.vertices) {
            file << " " << vi;
        }
        file << "\n";
    }

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool MeshLoader::save(const std::string& filePath, const modeling::Mesh& mesh) {
    FileFormat format = FileLoader::detectFormat(filePath);

    switch (format) {
        case FileFormat::OBJ: return saveOBJ(filePath, mesh);
        case FileFormat::STL: return saveSTL(filePath, mesh);
        case FileFormat::PLY: return savePLY(filePath, mesh);
        default:
            if (errorCallback_) {
                errorCallback_("Unsupported file format: " + filePath);
            }
            return false;
    }
}

void MeshLoader::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void MeshLoader::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

std::function<void(float)> AnimationLoader::progressCallback_;
std::function<void(const std::string&)> AnimationLoader::errorCallback_;

std::shared_ptr<animation::Skeleton> AnimationLoader::loadSkeleton(const std::string& filePath) {
    FileFormat format = FileLoader::detectFormat(filePath);

    if (format == FileFormat::PMX) {
        auto mesh = MeshLoader::loadPMX(filePath);
        if (mesh) {
            auto skeleton = std::make_shared<animation::Skeleton>();
            skeleton->setName(FileLoader::getFileName(filePath));
            return skeleton;
        }
    }

    return nullptr;
}

std::shared_ptr<animation::AnimationClip> AnimationLoader::loadAnimationClip(const std::string& filePath) {
    FileFormat format = FileLoader::detectFormat(filePath);

    if (format == FileFormat::VMD) {
        return loadVMD(filePath);
    }

    return nullptr;
}

std::shared_ptr<animation::AnimationClip> AnimationLoader::loadVMD(const std::string& filePath) {
    auto clip = std::make_shared<animation::AnimationClip>();
    clip->setName(FileLoader::getFileName(filePath));

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to open VMD file: " + filePath);
        }
        return nullptr;
    }

    char header[30];
    file.read(header, 30);

    if (std::string(header, 23) != "Vocaloid Motion Data") {
        if (errorCallback_) {
            errorCallback_("Invalid VMD file format");
        }
        return nullptr;
    }

    if (progressCallback_) {
        progressCallback_(0.5f);
    }

    uint32_t boneCount;
    file.read(reinterpret_cast<char*>(&boneCount), 4);

    for (uint32_t i = 0; i < boneCount; ++i) {
        char boneName[15];
        file.read(boneName, 15);

        float position[3], rotation[4];
        file.read(reinterpret_cast<char*>(position), 12);
        file.read(reinterpret_cast<char*>(rotation), 16);
    }

    uint32_t faceCount;
    file.read(reinterpret_cast<char*>(&faceCount), 4);

    for (uint32_t i = 0; i < faceCount; ++i) {
        char faceName[15];
        file.read(faceName, 15);

        float position[3], rotation[4];
        file.read(reinterpret_cast<char*>(position), 12);
        file.read(reinterpret_cast<char*>(rotation), 16);
    }

    uint32_t cameraCount;
    file.read(reinterpret_cast<char*>(&cameraCount), 4);

    for (uint32_t i = 0; i < cameraCount; ++i) {
        char cameraName[15];
        file.read(cameraName, 15);

        float position[3], rotation[4], distance, angle;
        file.read(reinterpret_cast<char*>(position), 12);
        file.read(reinterpret_cast<char*>(rotation), 16);
        file.read(reinterpret_cast<char*>(&distance), 4);
        file.read(reinterpret_cast<char*>(&angle), 4);
    }

    uint32_t lightCount;
    file.read(reinterpret_cast<char*>(&lightCount), 4);

    for (uint32_t i = 0; i < lightCount; ++i) {
        char lightName[15];
        file.read(lightName, 15);

        float position[3], rotation[4], color[3], distance, angle;
        file.read(reinterpret_cast<char*>(position), 12);
        file.read(reinterpret_cast<char*>(rotation), 16);
        file.read(reinterpret_cast<char*>(color), 12);
        file.read(reinterpret_cast<char*>(&distance), 4);
        file.read(reinterpret_cast<char*>(&angle), 4);
    }

    clip->setDuration(10.0f);

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return clip;
}

std::shared_ptr<animation::AnimationClip> AnimationLoader::loadBVH(const std::string& filePath) {
    auto clip = std::make_shared<animation::AnimationClip>();
    clip->setName(FileLoader::getFileName(filePath));

    std::ifstream file(filePath);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to open BVH file: " + filePath);
        }
        return nullptr;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("HIERARCHY") != std::string::npos) {
            break;
        }
    }

    if (progressCallback_) {
        progressCallback_(0.5f);
    }

    while (std::getline(file, line)) {
        if (line.find("MOTION") != std::string::npos) {
            break;
        }
    }

    clip->setDuration(10.0f);

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return clip;
}

bool AnimationLoader::saveVMD(const std::string& filePath, const animation::AnimationClip& clip) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create VMD file: " + filePath);
        }
        return false;
    }

    char header[30] = "Vocaloid Motion Data 0002";
    file.write(header, 30);

    uint32_t boneCount = 0;
    file.write(reinterpret_cast<const char*>(&boneCount), 4);

    uint32_t faceCount = 0;
    file.write(reinterpret_cast<const char*>(&faceCount), 4);

    uint32_t cameraCount = 0;
    file.write(reinterpret_cast<const char*>(&cameraCount), 4);

    uint32_t lightCount = 0;
    file.write(reinterpret_cast<const char*>(&lightCount), 4);

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool AnimationLoader::saveBVH(const std::string& filePath, const animation::AnimationClip& clip) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create BVH file: " + filePath);
        }
        return false;
    }

    file << "HIERARCHY\n";
    file << "ROOT Hips\n";
    file << "{\n";
    file << "  OFFSET 0.0 0.0 0.0\n";
    file << "  CHANNELS Xposition Yposition Zposition Xrotation Yrotation Zrotation\n";
    file << "}\n";
    file << "MOTION\n";
    file << "Frames: " << static_cast<int>(clip.getDuration() * 30) << "\n";
    file << "Frame Time: 0.0333333\n";

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

void AnimationLoader::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void AnimationLoader::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

std::function<void(float)> AudioLoader::progressCallback_;
std::function<void(const std::string&)> AudioLoader::errorCallback_;

std::shared_ptr<audio::AudioBuffer> AudioLoader::loadWAV(const std::string& filePath) {
    auto buffer = std::make_shared<audio::AudioBuffer>();

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to open WAV file: " + filePath);
        }
        return nullptr;
    }

    char riffHeader[4];
    file.read(riffHeader, 4);

    if (riffHeader[0] != 'R' || riffHeader[1] != 'I' || riffHeader[2] != 'F' || riffHeader[3] != 'F') {
        if (errorCallback_) {
            errorCallback_("Invalid WAV file format");
        }
        return nullptr;
    }

    uint32_t fileSize;
    file.read(reinterpret_cast<char*>(&fileSize), 4);

    char waveHeader[4];
    file.read(waveHeader, 4);

    char fmtHeader[4];
    file.read(fmtHeader, 4);

    uint32_t fmtSize;
    file.read(reinterpret_cast<char*>(&fmtSize), 4);

    uint16_t audioFormat;
    file.read(reinterpret_cast<char*>(&audioFormat), 2);

    uint16_t numChannels;
    file.read(reinterpret_cast<char*>(&numChannels), 2);

    uint32_t sampleRate;
    file.read(reinterpret_cast<char*>(&sampleRate), 4);

    uint32_t byteRate;
    file.read(reinterpret_cast<char*>(&byteRate), 4);

    uint16_t blockAlign;
    file.read(reinterpret_cast<char*>(&blockAlign), 2);

    uint16_t bitsPerSample;
    file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

    char dataHeader[4];
    file.read(dataHeader, 4);

    uint32_t dataSize;
    file.read(reinterpret_cast<char*>(&dataSize), 4);

    audio::AudioSpec spec;
    spec.sampleRate = static_cast<int>(sampleRate);
    spec.format = audio::AudioFormat::PCM16;

    if (numChannels == 1) {
        spec.channels = audio::AudioChannelLayout::Mono;
    } else if (numChannels == 2) {
        spec.channels = audio::AudioChannelLayout::Stereo;
    } else {
        spec.channels = audio::AudioChannelLayout::Stereo;
    }

    size_t frameCount = dataSize / (bitsPerSample / 8) / numChannels;
    buffer->allocate(spec, frameCount);

    std::vector<int16_t> samples(frameCount * numChannels);
    file.read(reinterpret_cast<char*>(samples.data()), dataSize);

    float* floatSamples = reinterpret_cast<float*>(buffer->getData());
    for (size_t i = 0; i < samples.size(); ++i) {
        floatSamples[i] = static_cast<float>(samples[i]) / 32768.0f;
    }

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return buffer;
}

std::shared_ptr<audio::AudioBuffer> AudioLoader::loadMP3(const std::string& filePath) {
    auto buffer = std::make_shared<audio::AudioBuffer>();

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return buffer;
}

std::shared_ptr<audio::AudioBuffer> AudioLoader::loadOGG(const std::string& filePath) {
    auto buffer = std::make_shared<audio::AudioBuffer>();

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return buffer;
}

std::shared_ptr<audio::AudioBuffer> AudioLoader::loadFLAC(const std::string& filePath) {
    auto buffer = std::make_shared<audio::AudioBuffer>();

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return buffer;
}

std::shared_ptr<audio::AudioBuffer> AudioLoader::load(const std::string& filePath) {
    FileFormat format = FileLoader::detectFormat(filePath);

    switch (format) {
        case FileFormat::WAV: return loadWAV(filePath);
        case FileFormat::MP3: return loadMP3(filePath);
        case FileFormat::OGG: return loadOGG(filePath);
        case FileFormat::FLAC: return loadFLAC(filePath);
        default:
            if (errorCallback_) {
                errorCallback_("Unsupported audio format: " + filePath);
            }
            return nullptr;
    }
}

bool AudioLoader::saveWAV(const std::string& filePath, const audio::AudioBuffer& buffer) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorCallback_) {
            errorCallback_("Failed to create WAV file: " + filePath);
        }
        return false;
    }

    const audio::AudioSpec& spec = buffer.getSpec();
    int numChannels = spec.getChannelCount();
    int bitsPerSample = spec.getBytesPerSample() * 8;

    file.write("RIFF", 4);

    uint32_t fileSize = 36 + buffer.getSizeInBytes();
    file.write(reinterpret_cast<const char*>(&fileSize), 4);

    file.write("WAVE", 4);
    file.write("fmt ", 4);

    uint32_t fmtSize = 16;
    file.write(reinterpret_cast<const char*>(&fmtSize), 4);

    uint16_t audioFormat = 1;
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);

    file.write(reinterpret_cast<const char*>(&numChannels), 2);

    uint32_t sampleRate = spec.sampleRate;
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);

    uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
    file.write(reinterpret_cast<const char*>(&byteRate), 4);

    uint16_t blockAlign = numChannels * bitsPerSample / 8;
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);

    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    file.write("data", 4);

    uint32_t dataSize = buffer.getSizeInBytes();
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    file.write(reinterpret_cast<const char*>(buffer.getData()), buffer.getSizeInBytes());

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool AudioLoader::saveFLAC(const std::string& filePath, const audio::AudioBuffer& buffer) {
    return false;
}

bool AudioLoader::save(const std::string& filePath, const audio::AudioBuffer& buffer) {
    FileFormat format = FileLoader::detectFormat(filePath);

    switch (format) {
        case FileFormat::WAV: return saveWAV(filePath, buffer);
        case FileFormat::FLAC: return saveFLAC(filePath, buffer);
        default:
            if (errorCallback_) {
                errorCallback_("Unsupported audio format: " + filePath);
            }
            return false;
    }
}

void AudioLoader::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void AudioLoader::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

std::function<void(float)> ImageLoader::progressCallback_;
std::function<void(const std::string&)> ImageLoader::errorCallback_;

std::vector<uint8_t> ImageLoader::loadPNG(const std::string& filePath, int& width, int& height, int& channels) {
    std::vector<uint8_t> data;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return data;
}

std::vector<uint8_t> ImageLoader::loadJPG(const std::string& filePath, int& width, int& height, int& channels) {
    std::vector<uint8_t> data;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return data;
}

std::vector<uint8_t> ImageLoader::loadEXR(const std::string& filePath, int& width, int& height, int& channels) {
    std::vector<uint8_t> data;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return data;
}

std::vector<uint8_t> ImageLoader::loadHDR(const std::string& filePath, int& width, int& height, int& channels) {
    std::vector<uint8_t> data;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return data;
}

std::vector<uint8_t> ImageLoader::loadTGA(const std::string& filePath, int& width, int& height, int& channels) {
    std::vector<uint8_t> data;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return data;
}

std::vector<uint8_t> ImageLoader::loadBMP(const std::string& filePath, int& width, int& height, int& channels) {
    std::vector<uint8_t> data;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return data;
}

std::vector<uint8_t> ImageLoader::load(const std::string& filePath, int& width, int& height, int& channels) {
    FileFormat format = FileLoader::detectFormat(filePath);

    switch (format) {
        case FileFormat::PNG: return loadPNG(filePath, width, height, channels);
        case FileFormat::JPG: return loadJPG(filePath, width, height, channels);
        case FileFormat::EXR: return loadEXR(filePath, width, height, channels);
        case FileFormat::HDR: return loadHDR(filePath, width, height, channels);
        case FileFormat::TGA: return loadTGA(filePath, width, height, channels);
        case FileFormat::BMP: return loadBMP(filePath, width, height, channels);
        default:
            if (errorCallback_) {
                errorCallback_("Unsupported image format: " + filePath);
            }
            return {};
    }
}

bool ImageLoader::savePNG(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels) {
    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool ImageLoader::saveJPG(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels) {
    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool ImageLoader::saveEXR(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels) {
    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool ImageLoader::saveHDR(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels) {
    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

bool ImageLoader::save(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels) {
    FileFormat format = FileLoader::detectFormat(filePath);

    switch (format) {
        case FileFormat::PNG: return savePNG(filePath, data, width, height, channels);
        case FileFormat::JPG: return saveJPG(filePath, data, width, height, channels);
        case FileFormat::EXR: return saveEXR(filePath, data, width, height, channels);
        case FileFormat::HDR: return saveHDR(filePath, data, width, height, channels);
        default:
            if (errorCallback_) {
                errorCallback_("Unsupported image format: " + filePath);
            }
            return false;
    }
}

void ImageLoader::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void ImageLoader::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

std::function<void(float)> SceneLoader::progressCallback_;
std::function<void(const std::string&)> SceneLoader::errorCallback_;

std::shared_ptr<SceneLoader::SceneNode> SceneLoader::loadScene(const std::string& filePath) {
    FileFormat format = FileLoader::detectFormat(filePath);

    if (format == FileFormat::GLTF || format == FileFormat::GLB) {
        auto nodes = loadGLTFScene(filePath);
        if (!nodes.empty()) {
            return nodes[0];
        }
    }

    return nullptr;
}

bool SceneLoader::saveScene(const std::string& filePath, const std::shared_ptr<SceneNode>& root) {
    return false;
}

std::vector<std::shared_ptr<SceneLoader::SceneNode>> SceneLoader::loadGLTFScene(const std::string& filePath) {
    std::vector<std::shared_ptr<SceneNode>> nodes;

    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return nodes;
}

bool SceneLoader::saveGLTFScene(const std::string& filePath, const std::vector<std::shared_ptr<SceneNode>>& nodes) {
    if (progressCallback_) {
        progressCallback_(1.0f);
    }

    return true;
}

void SceneLoader::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void SceneLoader::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

}
}