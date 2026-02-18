#include "test_data_generator.h"
#include "test_utils.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <random>

namespace v3d {
namespace test {

modeling::Vertex TestDataGenerator::createVertex(float x, float y, float z,
                                                float nx, float ny, float nz,
                                                float u, float v) {
    modeling::Vertex vertex;
    vertex.position = glm::vec3(x, y, z);
    vertex.normal = glm::vec3(nx, ny, nz);
    vertex.texCoord = glm::vec2(u, v);
    return vertex;
}

std::shared_ptr<modeling::Mesh> TestDataGenerator::createCubeMesh() {
    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName("Cube");

    std::vector<modeling::Vertex> vertices = {
        createVertex(-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f),
        createVertex( 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f),
        createVertex( 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f),
        createVertex(-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f),
        createVertex(-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f),
        createVertex( 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f),
        createVertex( 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f),
        createVertex(-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f),
        createVertex(-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f),
        createVertex( 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f),
        createVertex( 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f),
        createVertex(-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f),
        createVertex(-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f),
        createVertex( 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f),
        createVertex( 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f),
        createVertex(-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f),
        createVertex(-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f),
        createVertex(-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f),
        createVertex(-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f),
        createVertex(-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f),
        createVertex( 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f),
        createVertex( 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f),
        createVertex( 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f),
        createVertex( 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f)
    };

    for (const auto& vertex : vertices) {
        mesh->addVertex(vertex);
    }

    std::vector<std::vector<int>> faces = {
        {0, 1, 2}, {0, 2, 3},
        {4, 7, 6}, {4, 6, 5},
        {8, 9, 10}, {8, 10, 11},
        {12, 15, 14}, {12, 14, 13},
        {16, 17, 18}, {16, 18, 19},
        {20, 23, 22}, {20, 22, 21}
    };

    for (const auto& face : faces) {
        mesh->addFace(face);
    }

    mesh->calculateNormals();
    mesh->calculateTangents();
    mesh->calculateBoundingBox();

    return mesh;
}

std::shared_ptr<modeling::Mesh> TestDataGenerator::createSphereMesh(int segments) {
    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName("Sphere");

    const float radius = 0.5f;
    const int rings = segments;

    for (int ring = 0; ring <= rings; ++ring) {
        float theta = static_cast<float>(ring) / rings * glm::pi<float>();
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int seg = 0; seg <= segments; ++seg) {
            float phi = static_cast<float>(seg) / segments * 2.0f * glm::pi<float>();
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            float u = static_cast<float>(seg) / segments;
            float v = static_cast<float>(ring) / rings;

            modeling::Vertex vertex;
            vertex.position = glm::vec3(x * radius, y * radius, z * radius);
            vertex.normal = glm::normalize(vertex.position);
            vertex.texCoord = glm::vec2(u, v);
            mesh->addVertex(vertex);
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;

            mesh->addTriangle(current, next, current + 1);
            mesh->addTriangle(current + 1, next, next + 1);
        }
    }

    mesh->calculateNormals();
    mesh->calculateTangents();
    mesh->calculateBoundingBox();

    return mesh;
}

std::shared_ptr<modeling::Mesh> TestDataGenerator::createPlaneMesh(int subdivisions) {
    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName("Plane");

    const float size = 1.0f;
    const float halfSize = size / 2.0f;
    const float step = size / subdivisions;

    for (int y = 0; y <= subdivisions; ++y) {
        for (int x = 0; x <= subdivisions; ++x) {
            float px = -halfSize + x * step;
            float pz = -halfSize + y * step;
            float u = static_cast<float>(x) / subdivisions;
            float v = static_cast<float>(y) / subdivisions;

            modeling::Vertex vertex;
            vertex.position = glm::vec3(px, 0.0f, pz);
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.texCoord = glm::vec2(u, v);
            mesh->addVertex(vertex);
        }
    }

    for (int y = 0; y < subdivisions; ++y) {
        for (int x = 0; x < subdivisions; ++x) {
            int i0 = y * (subdivisions + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (subdivisions + 1);
            int i3 = i2 + 1;

            mesh->addTriangle(i0, i2, i1);
            mesh->addTriangle(i1, i2, i3);
        }
    }

    mesh->calculateNormals();
    mesh->calculateTangents();
    mesh->calculateBoundingBox();

    return mesh;
}

std::shared_ptr<modeling::Mesh> TestDataGenerator::createCylinderMesh(int segments, float height) {
    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName("Cylinder");

    const float radius = 0.5f;
    const float halfHeight = height / 2.0f;

    for (int i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) / segments * 2.0f * glm::pi<float>();
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        float u = static_cast<float>(i) / segments;

        modeling::Vertex topVertex;
        topVertex.position = glm::vec3(x, halfHeight, z);
        topVertex.normal = glm::vec3(x, 0.0f, z);
        topVertex.texCoord = glm::vec2(u, 1.0f);
        mesh->addVertex(topVertex);

        modeling::Vertex bottomVertex;
        bottomVertex.position = glm::vec3(x, -halfHeight, z);
        bottomVertex.normal = glm::vec3(x, 0.0f, z);
        bottomVertex.texCoord = glm::vec2(u, 0.0f);
        mesh->addVertex(bottomVertex);
    }

    for (int i = 0; i < segments; ++i) {
        int top0 = i * 2;
        int top1 = ((i + 1) % (segments + 1)) * 2;
        int bottom0 = i * 2 + 1;
        int bottom1 = ((i + 1) % (segments + 1)) * 2 + 1;

        mesh->addTriangle(top0, bottom0, top1);
        mesh->addTriangle(top1, bottom0, bottom1);
    }

    mesh->calculateNormals();
    mesh->calculateTangents();
    mesh->calculateBoundingBox();

    return mesh;
}

std::shared_ptr<modeling::Mesh> TestDataGenerator::createTorusMesh(int segments, int rings) {
    auto mesh = std::make_shared<modeling::Mesh>();
    mesh->setName("Torus");

    const float majorRadius = 0.5f;
    const float minorRadius = 0.2f;

    for (int ring = 0; ring <= rings; ++ring) {
        float u = static_cast<float>(ring) / rings * 2.0f * glm::pi<float>();
        float cosU = std::cos(u);
        float sinU = std::sin(u);

        for (int seg = 0; seg <= segments; ++seg) {
            float v = static_cast<float>(seg) / segments * 2.0f * glm::pi<float>();
            float cosV = std::cos(v);
            float sinV = std::sin(v);

            float x = (majorRadius + minorRadius * cosV) * cosU;
            float y = minorRadius * sinV;
            float z = (majorRadius + minorRadius * cosV) * sinU;

            float texU = static_cast<float>(seg) / segments;
            float texV = static_cast<float>(ring) / rings;

            modeling::Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.texCoord = glm::vec2(texU, texV);
            mesh->addVertex(vertex);
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;

            mesh->addTriangle(current, next, current + 1);
            mesh->addTriangle(current + 1, next, next + 1);
        }
    }

    mesh->calculateNormals();
    mesh->calculateTangents();
    mesh->calculateBoundingBox();

    return mesh;
}

std::shared_ptr<animation::Skeleton> TestDataGenerator::createSimpleSkeleton() {
    auto skeleton = std::make_shared<animation::Skeleton>();
    skeleton->setName("SimpleSkeleton");

    auto root = skeleton->createBone("Root");
    auto bone1 = skeleton->createBone("Bone1", root);
    auto bone2 = skeleton->createBone("Bone2", bone1);

    skeleton->setRootBone(root);
    skeleton->calculateBindPose();

    return skeleton;
}

std::shared_ptr<animation::Skeleton> TestDataGenerator::createHumanoidSkeleton() {
    auto skeleton = std::make_shared<animation::Skeleton>();
    skeleton->setName("HumanoidSkeleton");

    auto hips = skeleton->createBone("Hips");
    auto spine = skeleton->createBone("Spine", hips);
    auto chest = skeleton->createBone("Chest", spine);
    auto neck = skeleton->createBone("Neck", chest);
    auto head = skeleton->createBone("Head", neck);

    auto leftShoulder = skeleton->createBone("LeftShoulder", chest);
    auto leftArm = skeleton->createBone("LeftArm", leftShoulder);
    auto leftForeArm = skeleton->createBone("LeftForeArm", leftArm);
    auto leftHand = skeleton->createBone("LeftHand", leftForeArm);

    auto rightShoulder = skeleton->createBone("RightShoulder", chest);
    auto rightArm = skeleton->createBone("RightArm", rightShoulder);
    auto rightForeArm = skeleton->createBone("RightForeArm", rightArm);
    auto rightHand = skeleton->createBone("RightHand", rightForeArm);

    auto leftUpLeg = skeleton->createBone("LeftUpLeg", hips);
    auto leftLeg = skeleton->createBone("LeftLeg", leftUpLeg);
    auto leftFoot = skeleton->createBone("LeftFoot", leftLeg);

    auto rightUpLeg = skeleton->createBone("RightUpLeg", hips);
    auto rightLeg = skeleton->createBone("RightLeg", rightUpLeg);
    auto rightFoot = skeleton->createBone("RightFoot", rightLeg);

    skeleton->setRootBone(hips);
    skeleton->calculateBindPose();

    return skeleton;
}

std::shared_ptr<animation::Skeleton> TestDataGenerator::createChainSkeleton(int boneCount) {
    auto skeleton = std::make_shared<animation::Skeleton>();
    skeleton->setName("ChainSkeleton");

    animation::Bone* prevBone = nullptr;
    for (int i = 0; i < boneCount; ++i) {
        std::string boneName = "Bone" + std::to_string(i);
        auto bone = skeleton->createBone(boneName, prevBone);
        if (i == 0) {
            skeleton->setRootBone(bone);
        }
        prevBone = bone;
    }

    skeleton->calculateBindPose();

    return skeleton;
}

std::shared_ptr<animation::AnimationClip> TestDataGenerator::createSimpleAnimationClip() {
    auto clip = std::make_shared<animation::AnimationClip>();
    clip->setName("SimpleAnimation");
    clip->setDuration(1.0f);
    clip->setLooping(true);

    return clip;
}

std::shared_ptr<animation::AnimationClip> TestDataGenerator::createWalkingAnimationClip() {
    auto clip = std::make_shared<animation::AnimationClip>();
    clip->setName("WalkingAnimation");
    clip->setDuration(1.0f);
    clip->setLooping(true);

    return clip;
}

std::shared_ptr<audio::AudioBuffer> TestDataGenerator::createSilentBuffer(int sampleRate, float duration) {
    audio::AudioSpec spec;
    spec.sampleRate = sampleRate;
    spec.format = audio::AudioFormat::Float32;
    spec.channels = audio::AudioChannelLayout::Stereo;

    size_t frameCount = static_cast<size_t>(sampleRate * duration);
    auto buffer = std::make_shared<audio::AudioBuffer>(spec, frameCount);

    buffer->clear();

    return buffer;
}

std::shared_ptr<audio::AudioBuffer> TestDataGenerator::createSineWaveBuffer(float frequency, int sampleRate, float duration) {
    audio::AudioSpec spec;
    spec.sampleRate = sampleRate;
    spec.format = audio::AudioFormat::Float32;
    spec.channels = audio::AudioChannelLayout::Mono;

    size_t frameCount = static_cast<size_t>(sampleRate * duration);
    auto buffer = std::make_shared<audio::AudioBuffer>(spec, frameCount);

    float* data = static_cast<float*>(buffer->getData());
    for (size_t i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        data[i] = std::sin(2.0f * glm::pi<float>() * frequency * t);
    }

    return buffer;
}

std::shared_ptr<audio::AudioBuffer> TestDataGenerator::createNoiseBuffer(int sampleRate, float duration) {
    audio::AudioSpec spec;
    spec.sampleRate = sampleRate;
    spec.format = audio::AudioFormat::Float32;
    spec.channels = audio::AudioChannelLayout::Mono;

    size_t frameCount = static_cast<size_t>(sampleRate * duration);
    auto buffer = std::make_shared<audio::AudioBuffer>(spec, frameCount);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float* data = static_cast<float*>(buffer->getData());
    for (size_t i = 0; i < frameCount; ++i) {
        data[i] = dist(gen);
    }

    return buffer;
}

std::vector<uint8_t> TestDataGenerator::createRandomImageData(int width, int height, int channels) {
    std::vector<uint8_t> data;
    data.reserve(width * height * channels);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    for (int i = 0; i < width * height * channels; ++i) {
        data.push_back(static_cast<uint8_t>(dist(gen)));
    }

    return data;
}

std::vector<uint8_t> TestDataGenerator::createGradientImageData(int width, int height, int channels) {
    std::vector<uint8_t> data;
    data.reserve(width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t r = static_cast<uint8_t>((x * 255) / width);
            uint8_t g = static_cast<uint8_t>((y * 255) / height);
            uint8_t b = static_cast<uint8_t>(((x + y) * 255) / (width + height));
            uint8_t a = 255;

            if (channels >= 1) data.push_back(r);
            if (channels >= 2) data.push_back(g);
            if (channels >= 3) data.push_back(b);
            if (channels >= 4) data.push_back(a);
        }
    }

    return data;
}

std::vector<uint8_t> TestDataGenerator::createCheckerboardImageData(int width, int height, int channels, int tileSize) {
    std::vector<uint8_t> data;
    data.reserve(width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            bool white = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            uint8_t value = white ? 255 : 0;
            uint8_t a = 255;

            for (int c = 0; c < channels; ++c) {
                if (c < 3) {
                    data.push_back(value);
                } else {
                    data.push_back(a);
                }
            }
        }
    }

    return data;
}

std::string TestDataGenerator::createTestTextFile(const std::string& content) {
    std::string path = "test_temp/" + TestUtils::generateRandomString(10) + ".txt";
    std::ofstream file(path);
    file << content;
    file.close();
    return path;
}

std::string TestDataGenerator::createTestBinaryFile(const std::vector<uint8_t>& data) {
    std::string path = "test_temp/" + TestUtils::generateRandomString(10) + ".bin";
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    return path;
}

std::vector<float> TestDataGenerator::generateRandomFloats(size_t count, float min, float max) {
    std::vector<float> result;
    result.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);

    for (size_t i = 0; i < count; ++i) {
        result.push_back(dist(gen));
    }

    return result;
}

std::vector<int> TestDataGenerator::generateRandomInts(size_t count, int min, int max) {
    std::vector<int> result;
    result.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);

    for (size_t i = 0; i < count; ++i) {
        result.push_back(dist(gen));
    }

    return result;
}

std::vector<glm::vec3> TestDataGenerator::generateRandomVec3s(size_t count, float min, float max) {
    std::vector<glm::vec3> result;
    result.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);

    for (size_t i = 0; i < count; ++i) {
        result.emplace_back(dist(gen), dist(gen), dist(gen));
    }

    return result;
}

}
}
