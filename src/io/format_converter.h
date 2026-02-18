#pragma once

#include "file_loader.h"
#include "../modeling/mesh.h"
#include "../animation/animation_clip.h"
#include "../audio/audio_buffer.h"
#include <string>
#include <memory>
#include <functional>

namespace v3d {
namespace io {

struct ConversionOptions {
    float scale = 1.0f;
    bool flipUVs = false;
    bool flipWinding = false;
    bool generateNormals = false;
    bool generateTangents = false;
    bool optimizeMesh = false;
    int maxVertices = 65535;
    int maxBonesPerVertex = 4;
    bool compressTextures = true;
    int textureQuality = 85;
    bool mergeMaterials = false;
    bool removeUnusedMaterials = false;
};

struct AudioConversionOptions {
    int targetSampleRate = 44100;
    int targetBitDepth = 16;
    int targetChannels = 2;
    float quality = 0.9f;
    bool normalize = false;
    float normalizeLevel = -1.0f;
    bool fade = false;
    float fadeInDuration = 0.0f;
    float fadeOutDuration = 0.0f;
};

struct ConversionResult {
    bool success;
    std::string outputPath;
    std::string errorMessage;
    size_t originalSize;
    size_t convertedSize;
    float conversionTime;
};

class FormatConverter {
public:
    static ConversionResult convertMesh(const std::string& inputPath, const std::string& outputPath,
                                       const ConversionOptions& options = ConversionOptions());

    static ConversionResult convertAnimation(const std::string& inputPath, const std::string& outputPath,
                                            const ConversionOptions& options = ConversionOptions());

    static ConversionResult convertAudio(const std::string& inputPath, const std::string& outputPath,
                                       const AudioConversionOptions& options = AudioConversionOptions());

    static ConversionResult convertImage(const std::string& inputPath, const std::string& outputPath,
                                        int targetWidth = 0, int targetHeight = 0, int quality = 85);

    static ConversionResult convertScene(const std::string& inputPath, const std::string& outputPath,
                                        const ConversionOptions& options = ConversionOptions());

    static bool canConvert(FileFormat from, FileFormat to);
    static std::vector<FileFormat> getSupportedFormats();
    static std::vector<FileFormat> getTargetFormats(FileFormat sourceFormat);

    static void setProgressCallback(std::function<void(float)> callback);
    static void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    static std::shared_ptr<modeling::Mesh> loadAndProcessMesh(const std::string& inputPath,
                                                               const ConversionOptions& options);
    static bool saveMesh(const std::shared_ptr<modeling::Mesh>& mesh, const std::string& outputPath,
                        const ConversionOptions& options);

    static std::shared_ptr<audio::AudioBuffer> loadAndProcessAudio(const std::string& inputPath,
                                                                   const AudioConversionOptions& options);
    static bool saveAudio(const std::shared_ptr<audio::AudioBuffer>& buffer, const std::string& outputPath,
                         const AudioConversionOptions& options);

    static std::vector<uint8_t> loadAndProcessImage(const std::string& inputPath,
                                                    int& width, int& height, int& channels,
                                                    int targetWidth, int targetHeight, int quality);
    static bool saveImage(const std::vector<uint8_t>& data, const std::string& outputPath,
                         int width, int height, int channels, int quality);

    static std::function<void(float)> progressCallback_;
    static std::function<void(const std::string&)> errorCallback_;
};

class BatchConverter {
public:
    BatchConverter();

    void addConversion(const std::string& inputPath, const std::string& outputPath,
                      const ConversionOptions& options = ConversionOptions());

    void addAudioConversion(const std::string& inputPath, const std::string& outputPath,
                           const AudioConversionOptions& options = AudioConversionOptions());

    void addImageConversion(const std::string& inputPath, const std::string& outputPath,
                           int targetWidth = 0, int targetHeight = 0, int quality = 85);

    std::vector<ConversionResult> execute();
    void cancel();

    float getProgress() const;
    int getCompletedCount() const;
    int getTotalCount() const;

    void setMaxConcurrentConversions(int maxConversions);
    int getMaxConcurrentConversions() const;

    void setProgressCallback(std::function<void(float, const std::string&)> callback);
    void setErrorCallback(std::function<void(const std::string&, const std::string&)> callback);

private:
    struct ConversionTask {
        std::string inputPath;
        std::string outputPath;
        enum class Type { Mesh, Audio, Image, Scene } type;
        ConversionOptions meshOptions;
        AudioConversionOptions audioOptions;
        int imageTargetWidth;
        int imageTargetHeight;
        int imageQuality;
    };

    std::vector<ConversionTask> tasks_;
    std::vector<ConversionResult> results_;
    std::atomic<bool> cancelled_;
    std::atomic<float> progress_;
    std::atomic<int> completedCount_;
    std::atomic<int> maxConcurrentConversions_;

    std::function<void(float, const std::string&)> progressCallback_;
    std::function<void(const std::string&, const std::string&)> errorCallback_;
};

}
}
