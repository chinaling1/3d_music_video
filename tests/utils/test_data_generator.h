#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../modeling/mesh.h"
#include "../animation/skeleton.h"
#include "../animation/animation_clip.h"
#include "../audio/audio_buffer.h"
#include <glm/glm.hpp>

namespace v3d {
namespace test {

class TestDataGenerator {
public:
    static std::shared_ptr<modeling::Mesh> createCubeMesh();
    static std::shared_ptr<modeling::Mesh> createSphereMesh(int segments = 16);
    static std::shared_ptr<modeling::Mesh> createPlaneMesh(int subdivisions = 10);
    static std::shared_ptr<modeling::Mesh> createCylinderMesh(int segments = 16, float height = 1.0f);
    static std::shared_ptr<modeling::Mesh> createTorusMesh(int segments = 16, int rings = 16);

    static std::shared_ptr<animation::Skeleton> createSimpleSkeleton();
    static std::shared_ptr<animation::Skeleton> createHumanoidSkeleton();
    static std::shared_ptr<animation::Skeleton> createChainSkeleton(int boneCount);

    static std::shared_ptr<animation::AnimationClip> createSimpleAnimationClip();
    static std::shared_ptr<animation::AnimationClip> createWalkingAnimationClip();

    static std::shared_ptr<audio::AudioBuffer> createSilentBuffer(int sampleRate = 44100, float duration = 1.0f);
    static std::shared_ptr<audio::AudioBuffer> createSineWaveBuffer(float frequency = 440.0f, 
                                                                   int sampleRate = 44100, 
                                                                   float duration = 1.0f);
    static std::shared_ptr<audio::AudioBuffer> createNoiseBuffer(int sampleRate = 44100, float duration = 1.0f);

    static std::vector<uint8_t> createRandomImageData(int width, int height, int channels);
    static std::vector<uint8_t> createGradientImageData(int width, int height, int channels);
    static std::vector<uint8_t> createCheckerboardImageData(int width, int height, int channels, int tileSize = 8);

    static std::string createTestTextFile(const std::string& content);
    static std::string createTestBinaryFile(const std::vector<uint8_t>& data);

    static std::vector<float> generateRandomFloats(size_t count, float min = -1.0f, float max = 1.0f);
    static std::vector<int> generateRandomInts(size_t count, int min = 0, int max = 100);
    static std::vector<glm::vec3> generateRandomVec3s(size_t count, float min = -1.0f, float max = 1.0f);

private:
    static modeling::Vertex createVertex(float x, float y, float z, 
                                       float nx = 0.0f, float ny = 0.0f, float nz = 1.0f,
                                       float u = 0.0f, float v = 0.0f);
};

}
}
