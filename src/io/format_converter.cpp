#include "format_converter.h"
#include "file_saver.h"
#include <chrono>
#include <algorithm>
#include <cmath>
#include <thread>
#include <future>

namespace v3d {
namespace io {

std::function<void(float)> FormatConverter::progressCallback_;
std::function<void(const std::string&)> FormatConverter::errorCallback_;

ConversionResult FormatConverter::convertMesh(const std::string& inputPath, const std::string& outputPath,
                                             const ConversionOptions& options) {
    auto startTime = std::chrono::high_resolution_clock::now();
    ConversionResult result;
    result.success = false;
    result.outputPath = outputPath;
    result.originalSize = FileLoader::getFileSize(inputPath);

    try {
        FileFormat inputFormat = FileLoader::detectFormat(inputPath);
        FileFormat outputFormat = FileLoader::detectFormat(outputPath);

        if (!canConvert(inputFormat, outputFormat)) {
            result.errorMessage = "Unsupported conversion from " + FileLoader::getFormatExtension(inputFormat) +
                                 " to " + FileLoader::getFormatExtension(outputFormat);
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(0.1f);

        auto mesh = loadAndProcessMesh(inputPath, options);
        if (!mesh) {
            result.errorMessage = "Failed to load mesh from " + inputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(0.7f);

        bool saved = saveMesh(mesh, outputPath, options);
        if (!saved) {
            result.errorMessage = "Failed to save mesh to " + outputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(1.0f);

        result.success = true;
        result.convertedSize = FileLoader::getFileSize(outputPath);

    } catch (const std::exception& e) {
        result.errorMessage = std::string("Conversion error: ") + e.what();
        if (errorCallback_) errorCallback_(result.errorMessage);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.conversionTime = std::chrono::duration<float>(endTime - startTime).count();

    return result;
}

ConversionResult FormatConverter::convertAnimation(const std::string& inputPath, const std::string& outputPath,
                                                  const ConversionOptions& options) {
    auto startTime = std::chrono::high_resolution_clock::now();
    ConversionResult result;
    result.success = false;
    result.outputPath = outputPath;
    result.originalSize = FileLoader::getFileSize(inputPath);

    try {
        FileFormat inputFormat = FileLoader::detectFormat(inputPath);
        FileFormat outputFormat = FileLoader::detectFormat(outputPath);

        if (!canConvert(inputFormat, outputFormat)) {
            result.errorMessage = "Unsupported animation conversion";
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(0.2f);

        auto clip = AnimationLoader::loadAnimationClip(inputPath);
        if (!clip) {
            result.errorMessage = "Failed to load animation from " + inputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(0.6f);

        bool saved = false;
        if (outputFormat == FileFormat::VMD) {
            saved = AnimationLoader::saveVMD(outputPath, *clip);
        } else if (outputFormat == FileFormat::BVH) {
            saved = AnimationLoader::saveBVH(outputPath, *clip);
        }

        if (!saved) {
            result.errorMessage = "Failed to save animation to " + outputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(1.0f);

        result.success = true;
        result.convertedSize = FileLoader::getFileSize(outputPath);

    } catch (const std::exception& e) {
        result.errorMessage = std::string("Animation conversion error: ") + e.what();
        if (errorCallback_) errorCallback_(result.errorMessage);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.conversionTime = std::chrono::duration<float>(endTime - startTime).count();

    return result;
}

ConversionResult FormatConverter::convertAudio(const std::string& inputPath, const std::string& outputPath,
                                              const AudioConversionOptions& options) {
    auto startTime = std::chrono::high_resolution_clock::now();
    ConversionResult result;
    result.success = false;
    result.outputPath = outputPath;
    result.originalSize = FileLoader::getFileSize(inputPath);

    try {
        if (progressCallback_) progressCallback_(0.1f);

        auto buffer = loadAndProcessAudio(inputPath, options);
        if (!buffer) {
            result.errorMessage = "Failed to load audio from " + inputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(0.7f);

        bool saved = saveAudio(buffer, outputPath, options);
        if (!saved) {
            result.errorMessage = "Failed to save audio to " + outputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(1.0f);

        result.success = true;
        result.convertedSize = FileLoader::getFileSize(outputPath);

    } catch (const std::exception& e) {
        result.errorMessage = std::string("Audio conversion error: ") + e.what();
        if (errorCallback_) errorCallback_(result.errorMessage);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.conversionTime = std::chrono::duration<float>(endTime - startTime).count();

    return result;
}

ConversionResult FormatConverter::convertImage(const std::string& inputPath, const std::string& outputPath,
                                              int targetWidth, int targetHeight, int quality) {
    auto startTime = std::chrono::high_resolution_clock::now();
    ConversionResult result;
    result.success = false;
    result.outputPath = outputPath;
    result.originalSize = FileLoader::getFileSize(inputPath);

    try {
        int width, height, channels;
        if (progressCallback_) progressCallback_(0.1f);

        auto data = loadAndProcessImage(inputPath, width, height, channels, targetWidth, targetHeight, quality);
        if (data.empty()) {
            result.errorMessage = "Failed to load image from " + inputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(0.7f);

        bool saved = saveImage(data, outputPath, width, height, channels, quality);
        if (!saved) {
            result.errorMessage = "Failed to save image to " + outputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(1.0f);

        result.success = true;
        result.convertedSize = FileLoader::getFileSize(outputPath);

    } catch (const std::exception& e) {
        result.errorMessage = std::string("Image conversion error: ") + e.what();
        if (errorCallback_) errorCallback_(result.errorMessage);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.conversionTime = std::chrono::duration<float>(endTime - startTime).count();

    return result;
}

ConversionResult FormatConverter::convertScene(const std::string& inputPath, const std::string& outputPath,
                                              const ConversionOptions& options) {
    auto startTime = std::chrono::high_resolution_clock::now();
    ConversionResult result;
    result.success = false;
    result.outputPath = outputPath;
    result.originalSize = FileLoader::getFileSize(inputPath);

    try {
        if (progressCallback_) progressCallback_(0.1f);

        auto scene = SceneLoader::loadScene(inputPath);
        if (!scene) {
            result.errorMessage = "Failed to load scene from " + inputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(0.6f);

        bool saved = SceneLoader::saveScene(outputPath, scene);
        if (!saved) {
            result.errorMessage = "Failed to save scene to " + outputPath;
            if (errorCallback_) errorCallback_(result.errorMessage);
            return result;
        }

        if (progressCallback_) progressCallback_(1.0f);

        result.success = true;
        result.convertedSize = FileLoader::getFileSize(outputPath);

    } catch (const std::exception& e) {
        result.errorMessage = std::string("Scene conversion error: ") + e.what();
        if (errorCallback_) errorCallback_(result.errorMessage);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.conversionTime = std::chrono::duration<float>(endTime - startTime).count();

    return result;
}

bool FormatConverter::canConvert(FileFormat from, FileFormat to) {
    if (from == FileFormat::Unknown || to == FileFormat::Unknown) return false;

    std::vector<FileFormat> meshFormats = {FileFormat::OBJ, FileFormat::FBX, FileFormat::GLTF, FileFormat::GLB,
                                           FileFormat::DAE, FileFormat::STL, FileFormat::PLY, FileFormat::PMX};

    std::vector<FileFormat> audioFormats = {FileFormat::WAV, FileFormat::MP3, FileFormat::OGG, FileFormat::FLAC};

    std::vector<FileFormat> imageFormats = {FileFormat::PNG, FileFormat::JPG, FileFormat::EXR, FileFormat::HDR,
                                            FileFormat::TGA, FileFormat::BMP};

    std::vector<FileFormat> animationFormats = {FileFormat::VMD, FileFormat::BVH};

    auto isMeshFormat = [&meshFormats](FileFormat f) {
        return std::find(meshFormats.begin(), meshFormats.end(), f) != meshFormats.end();
    };
    auto isAudioFormat = [&audioFormats](FileFormat f) {
        return std::find(audioFormats.begin(), audioFormats.end(), f) != audioFormats.end();
    };
    auto isImageFormat = [&imageFormats](FileFormat f) {
        return std::find(imageFormats.begin(), imageFormats.end(), f) != imageFormats.end();
    };
    auto isAnimationFormat = [&animationFormats](FileFormat f) {
        return std::find(animationFormats.begin(), animationFormats.end(), f) != animationFormats.end();
    };

    if (isMeshFormat(from) && isMeshFormat(to)) return true;
    if (isAudioFormat(from) && isAudioFormat(to)) return true;
    if (isImageFormat(from) && isImageFormat(to)) return true;
    if (isAnimationFormat(from) && isAnimationFormat(to)) return true;

    return false;
}

std::vector<FileFormat> FormatConverter::getSupportedFormats() {
    return {FileFormat::OBJ, FileFormat::FBX, FileFormat::GLTF, FileFormat::GLB, FileFormat::DAE,
            FileFormat::STL, FileFormat::PLY, FileFormat::PMX, FileFormat::VMD, FileFormat::BVH,
            FileFormat::WAV, FileFormat::MP3, FileFormat::OGG, FileFormat::FLAC,
            FileFormat::PNG, FileFormat::JPG, FileFormat::EXR, FileFormat::HDR, FileFormat::TGA, FileFormat::BMP};
}

std::vector<FileFormat> FormatConverter::getTargetFormats(FileFormat sourceFormat) {
    std::vector<FileFormat> allFormats = getSupportedFormats();
    std::vector<FileFormat> targets;

    for (auto format : allFormats) {
        if (canConvert(sourceFormat, format)) {
            targets.push_back(format);
        }
    }

    return targets;
}

void FormatConverter::setProgressCallback(std::function<void(float)> callback) {
    progressCallback_ = callback;
}

void FormatConverter::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

std::shared_ptr<modeling::Mesh> FormatConverter::loadAndProcessMesh(const std::string& inputPath,
                                                                   const ConversionOptions& options) {
    auto mesh = MeshLoader::load(inputPath);
    if (!mesh) return nullptr;

    if (options.flipUVs) {
        mesh->flipUVs();
    }

    if (options.flipWinding) {
        mesh->flipFaces();
    }

    if (options.generateNormals) {
        mesh->calculateNormals();
    }

    if (options.generateTangents) {
        mesh->calculateTangents();
    }

    if (options.optimizeMesh) {
        mesh->optimize();
    }

    if (options.scale != 1.0f) {
        mesh->scale(glm::vec3(options.scale));
    }

    return mesh;
}

bool FormatConverter::saveMesh(const std::shared_ptr<modeling::Mesh>& mesh, const std::string& outputPath,
                               const ConversionOptions& options) {
    return MeshLoader::save(outputPath, *mesh);
}

std::shared_ptr<audio::AudioBuffer> FormatConverter::loadAndProcessAudio(const std::string& inputPath,
                                                                          const AudioConversionOptions& options) {
    auto buffer = AudioLoader::load(inputPath);
    if (!buffer) return nullptr;

    const auto& spec = buffer->getSpec();

    if (options.targetSampleRate > 0 && options.targetSampleRate != spec.sampleRate) {
        buffer->resample(options.targetSampleRate);
    }

    if (options.targetChannels > 0 && options.targetChannels != spec.getChannelCount()) {
        audio::AudioChannelLayout targetLayout;
        if (options.targetChannels == 1) {
            targetLayout = audio::AudioChannelLayout::Mono;
        } else if (options.targetChannels == 2) {
            targetLayout = audio::AudioChannelLayout::Stereo;
        } else if (options.targetChannels == 4) {
            targetLayout = audio::AudioChannelLayout::Quad;
        } else if (options.targetChannels == 6) {
            targetLayout = audio::AudioChannelLayout::Surround51;
        } else if (options.targetChannels == 8) {
            targetLayout = audio::AudioChannelLayout::Surround71;
        } else {
            targetLayout = audio::AudioChannelLayout::Stereo;
        }
        buffer->convertChannels(targetLayout);
    }

    if (options.normalize) {
        buffer->applyGain(1.0f);
    }

    if (options.fade) {
        size_t fadeInFrames = static_cast<size_t>(options.fadeInDuration * spec.sampleRate);
        size_t fadeOutFrames = static_cast<size_t>(options.fadeOutDuration * spec.sampleRate);
        buffer->applyFadeIn(fadeInFrames);
        buffer->applyFadeOut(fadeOutFrames);
    }

    return buffer;
}

bool FormatConverter::saveAudio(const std::shared_ptr<audio::AudioBuffer>& buffer, const std::string& outputPath,
                                const AudioConversionOptions& options) {
    return AudioLoader::save(outputPath, *buffer);
}

std::vector<uint8_t> FormatConverter::loadAndProcessImage(const std::string& inputPath,
                                                          int& width, int& height, int& channels,
                                                          int targetWidth, int targetHeight, int quality) {
    auto data = ImageLoader::load(inputPath, width, height, channels);
    if (data.empty()) return data;

    if (targetWidth > 0 && targetHeight > 0 && (width != targetWidth || height != targetHeight)) {
        float scaleX = static_cast<float>(targetWidth) / width;
        float scaleY = static_cast<float>(targetHeight) / height;

        std::vector<uint8_t> resizedData(targetWidth * targetHeight * channels);

        for (int y = 0; y < targetHeight; ++y) {
            for (int x = 0; x < targetWidth; ++x) {
                float srcX = x / scaleX;
                float srcY = y / scaleY;

                int x0 = static_cast<int>(srcX);
                int y0 = static_cast<int>(srcY);
                int x1 = std::min(x0 + 1, width - 1);
                int y1 = std::min(y0 + 1, height - 1);

                float fx = srcX - x0;
                float fy = srcY - y0;

                for (int c = 0; c < channels; ++c) {
                    float v00 = data[(y0 * width + x0) * channels + c];
                    float v10 = data[(y0 * width + x1) * channels + c];
                    float v01 = data[(y1 * width + x0) * channels + c];
                    float v11 = data[(y1 * width + x1) * channels + c];

                    float v0 = v00 * (1.0f - fx) + v10 * fx;
                    float v1 = v01 * (1.0f - fx) + v11 * fx;
                    float v = v0 * (1.0f - fy) + v1 * fy;

                    resizedData[(y * targetWidth + x) * channels + c] = static_cast<uint8_t>(std::round(v));
                }
            }
        }

        width = targetWidth;
        height = targetHeight;
        return resizedData;
    }

    return data;
}

bool FormatConverter::saveImage(const std::vector<uint8_t>& data, const std::string& outputPath,
                               int width, int height, int channels, int quality) {
    return ImageLoader::save(outputPath, data, width, height, channels);
}

BatchConverter::BatchConverter()
    : cancelled_(false)
    , progress_(0.0f)
    , completedCount_(0)
    , maxConcurrentConversions_(4) {
}

void BatchConverter::addConversion(const std::string& inputPath, const std::string& outputPath,
                                  const ConversionOptions& options) {
    ConversionTask task;
    task.inputPath = inputPath;
    task.outputPath = outputPath;
    task.type = ConversionTask::Type::Mesh;
    task.meshOptions = options;
    tasks_.push_back(task);
}

void BatchConverter::addAudioConversion(const std::string& inputPath, const std::string& outputPath,
                                        const AudioConversionOptions& options) {
    ConversionTask task;
    task.inputPath = inputPath;
    task.outputPath = outputPath;
    task.type = ConversionTask::Type::Audio;
    task.audioOptions = options;
    tasks_.push_back(task);
}

void BatchConverter::addImageConversion(const std::string& inputPath, const std::string& outputPath,
                                       int targetWidth, int targetHeight, int quality) {
    ConversionTask task;
    task.inputPath = inputPath;
    task.outputPath = outputPath;
    task.type = ConversionTask::Type::Image;
    task.imageTargetWidth = targetWidth;
    task.imageTargetHeight = targetHeight;
    task.imageQuality = quality;
    tasks_.push_back(task);
}

std::vector<ConversionResult> BatchConverter::execute() {
    results_.clear();
    results_.reserve(tasks_.size());
    cancelled_ = false;
    completedCount_ = 0;
    progress_ = 0.0f;

    std::vector<std::future<ConversionResult>> futures;
    int activeConversions = 0;
    size_t taskIndex = 0;

    while (taskIndex < tasks_.size() || activeConversions > 0) {
        if (cancelled_) break;

        while (activeConversions < maxConcurrentConversions_ && taskIndex < tasks_.size()) {
            const auto& task = tasks_[taskIndex];

            futures.push_back(std::async(std::launch::async, [task, this]() -> ConversionResult {
                FormatConverter::setProgressCallback([this, task](float progress) {
                    float totalProgress = (completedCount_ + progress) / tasks_.size();
                    progress_ = totalProgress;
                    if (progressCallback_) progressCallback_(totalProgress, task.inputPath);
                });

                FormatConverter::setErrorCallback([this, task](const std::string& error) {
                    if (errorCallback_) errorCallback_(task.inputPath, error);
                });

                switch (task.type) {
                    case ConversionTask::Type::Mesh:
                        return FormatConverter::convertMesh(task.inputPath, task.outputPath, task.meshOptions);
                    case ConversionTask::Type::Audio:
                        return FormatConverter::convertAudio(task.inputPath, task.outputPath, task.audioOptions);
                    case ConversionTask::Type::Image:
                        return FormatConverter::convertImage(task.inputPath, task.outputPath,
                                                            task.imageTargetWidth, task.imageTargetHeight,
                                                            task.imageQuality);
                    default:
                        ConversionResult result;
                        result.success = false;
                        result.errorMessage = "Unknown conversion type";
                        return result;
                }
            }));

            activeConversions++;
            taskIndex++;
        }

        for (auto it = futures.begin(); it != futures.end(); ) {
            if (it->wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
                results_.push_back(it->get());
                completedCount_++;
                activeConversions--;
                it = futures.erase(it);
            } else {
                ++it;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (auto& future : futures) {
        future.wait();
        results_.push_back(future.get());
    }

    return results_;
}

void BatchConverter::cancel() {
    cancelled_ = true;
}

float BatchConverter::getProgress() const {
    return progress_;
}

int BatchConverter::getCompletedCount() const {
    return completedCount_;
}

int BatchConverter::getTotalCount() const {
    return static_cast<int>(tasks_.size());
}

void BatchConverter::setMaxConcurrentConversions(int maxConversions) {
    maxConcurrentConversions_ = std::max(1, maxConversions);
}

int BatchConverter::getMaxConcurrentConversions() const {
    return maxConcurrentConversions_;
}

void BatchConverter::setProgressCallback(std::function<void(float, const std::string&)> callback) {
    progressCallback_ = callback;
}

void BatchConverter::setErrorCallback(std::function<void(const std::string&, const std::string&)> callback) {
    errorCallback_ = callback;
}

}
}
