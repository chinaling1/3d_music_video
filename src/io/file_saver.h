#pragma once

#include "file_loader.h"
#include "../modeling/mesh.h"
#include "../animation/skeleton.h"
#include "../animation/animation_clip.h"
#include "../audio/audio_buffer.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace v3d {
namespace io {

class MeshLoader {
public:
    static std::shared_ptr<modeling::Mesh> loadOBJ(const std::string& filePath);
    static std::shared_ptr<modeling::Mesh> loadFBX(const std::string& filePath);
    static std::shared_ptr<modeling::Mesh> loadGLTF(const std::string& filePath);
    static std::shared_ptr<modeling::Mesh> loadGLB(const std::string& filePath);
    static std::shared_ptr<modeling::Mesh> loadDAE(const std::string& filePath);
    static std::shared_ptr<modeling::Mesh> loadSTL(const std::string& filePath);
    static std::shared_ptr<modeling::Mesh> loadPLY(const std::string& filePath);
    static std::shared_ptr<modeling::Mesh> loadPMX(const std::string& filePath);

    static std::shared_ptr<modeling::Mesh> load(const std::string& filePath);

    static bool saveOBJ(const std::string& filePath, const modeling::Mesh& mesh);
    static bool saveSTL(const std::string& filePath, const modeling::Mesh& mesh);
    static bool savePLY(const std::string& filePath, const modeling::Mesh& mesh);

    static bool save(const std::string& filePath, const modeling::Mesh& mesh);

    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

class AnimationLoader {
public:
    static std::shared_ptr<animation::Skeleton> loadSkeleton(const std::string& filePath);
    static std::shared_ptr<animation::AnimationClip> loadAnimationClip(const std::string& filePath);

    static std::shared_ptr<animation::AnimationClip> loadVMD(const std::string& filePath);
    static std::shared_ptr<animation::AnimationClip> loadBVH(const std::string& filePath);

    static bool saveVMD(const std::string& filePath, const animation::AnimationClip& clip);
    static bool saveBVH(const std::string& filePath, const animation::AnimationClip& clip);

    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

class AudioLoader {
public:
    static std::shared_ptr<audio::AudioBuffer> loadWAV(const std::string& filePath);
    static std::shared_ptr<audio::AudioBuffer> loadMP3(const std::string& filePath);
    static std::shared_ptr<audio::AudioBuffer> loadOGG(const std::string& filePath);
    static std::shared_ptr<audio::AudioBuffer> loadFLAC(const std::string& filePath);

    static std::shared_ptr<audio::AudioBuffer> load(const std::string& filePath);

    static bool saveWAV(const std::string& filePath, const audio::AudioBuffer& buffer);
    static bool saveFLAC(const std::string& filePath, const audio::AudioBuffer& buffer);

    static bool save(const std::string& filePath, const audio::AudioBuffer& buffer);

    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

class ImageLoader {
public:
    static std::vector<uint8_t> loadPNG(const std::string& filePath, int& width, int& height, int& channels);
    static std::vector<uint8_t> loadJPG(const std::string& filePath, int& width, int& height, int& channels);
    static std::vector<uint8_t> loadEXR(const std::string& filePath, int& width, int& height, int& channels);
    static std::vector<uint8_t> loadHDR(const std::string& filePath, int& width, int& height, int& channels);
    static std::vector<uint8_t> loadTGA(const std::string& filePath, int& width, int& height, int& channels);
    static std::vector<uint8_t> loadBMP(const std::string& filePath, int& width, int& height, int& channels);

    static std::vector<uint8_t> load(const std::string& filePath, int& width, int& height, int& channels);

    static bool savePNG(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels);
    static bool saveJPG(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels);
    static bool saveEXR(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels);
    static bool saveHDR(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels);

    static bool save(const std::string& filePath, const std::vector<uint8_t>& data, int width, int height, int channels);

    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

class SceneLoader {
public:
    struct SceneNode {
        std::string name;
        std::string meshPath;
        std::shared_ptr<modeling::Mesh> mesh;
        glm::mat4 transform;
        std::vector<std::shared_ptr<SceneNode>> children;
        std::shared_ptr<animation::Skeleton> skeleton;
        std::string animationPath;
        std::shared_ptr<animation::AnimationClip> animation;
    };

    static std::shared_ptr<SceneNode> loadScene(const std::string& filePath);
    static bool saveScene(const std::string& filePath, const std::shared_ptr<SceneNode>& root);

    static std::vector<std::shared_ptr<SceneNode>> loadGLTFScene(const std::string& filePath);
    static bool saveGLTFScene(const std::string& filePath, const std::vector<std::shared_ptr<SceneNode>>& nodes);

    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

}
}