/**
 * @file audio_converter.h
 * @brief 音频格式转换器 - 支持多种音频格式转换和压缩
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

namespace v3d {
namespace app {

enum class AudioCodec {
    PCM,
    MP3,
    AAC,
    Vorbis,
    FLAC,
    Opus,
    WAV
};

enum class AudioSampleRate {
    SR_22050 = 22050,
    SR_44100 = 44100,
    SR_48000 = 48000,
    SR_96000 = 96000
};

enum class AudioBitDepth {
    Bit16 = 16,
    Bit24 = 24,
    Bit32 = 32,
    Float32 = 32
};

enum class AudioChannels {
    Mono = 1,
    Stereo = 2,
    Surround51 = 6,
    Surround71 = 8
};

struct AudioConvertSettings {
    AudioCodec codec = AudioCodec::AAC;
    AudioSampleRate sampleRate = AudioSampleRate::SR_48000;
    AudioBitDepth bitDepth = AudioBitDepth::Bit16;
    AudioChannels channels = AudioChannels::Stereo;
    
    int bitrate = 192;
    int quality = 5;
    bool normalize = false;
    float normalizeTarget = 0.9f;
    bool trimSilence = false;
    float silenceThreshold = -60.0f;
    bool fadeIn = false;
    float fadeInDuration = 0.0f;
    bool fadeOut = false;
    float fadeOutDuration = 0.0f;
    float startTime = 0.0f;
    float endTime = 0.0f;
    bool useTimeRange = false;
};

struct AudioConvertProgress {
    float progress;
    std::string currentStage;
    int currentPass;
    int totalPasses;
    float estimatedTimeRemaining;
};

class AudioConverter {
public:
    using ProgressCallback = std::function<void(const AudioConvertProgress&)>;
    using CompletionCallback = std::function<void(bool, const std::string&)>;

    AudioConverter();
    ~AudioConverter();
    
    static AudioConverter& getInstance();
    
    bool convert(const std::string& inputPath, const std::string& outputPath,
                 const AudioConvertSettings& settings);
    
    void convertAsync(const std::string& inputPath, const std::string& outputPath,
                      const AudioConvertSettings& settings,
                      ProgressCallback progressCallback,
                      CompletionCallback completionCallback);
    
    void cancel();
    bool isConverting() const;
    float getProgress() const;
    
    std::vector<std::string> getSupportedInputFormats() const;
    std::vector<std::string> getSupportedOutputFormats() const;
    
    bool getAudioInfo(const std::string& filePath,
                      int& sampleRate, int& channels, int& bitDepth,
                      float& duration);
    
    static std::string codecToString(AudioCodec codec);
    static AudioCodec stringToCodec(const std::string& str);
    static std::string getCodecExtension(AudioCodec codec);
    
    void setToolPath(const std::string& toolName, const std::string& path);
    std::string getToolPath(const std::string& toolName) const;
    
    struct Preset {
        std::string name;
        std::string description;
        AudioConvertSettings settings;
    };
    
    std::vector<Preset> getPresets() const;
    void addPreset(const Preset& preset);
    void removePreset(const std::string& name);
    
    bool batchConvert(const std::vector<std::string>& inputPaths,
                      const std::string& outputDirectory,
                      const AudioConvertSettings& settings,
                      ProgressCallback progressCallback);

private:
    void convertThread(const std::string& inputPath, const std::string& outputPath,
                       AudioConvertSettings settings,
                       ProgressCallback progressCallback,
                       CompletionCallback completionCallback);
    
    bool convertToWav(const std::string& inputPath, const std::string& wavPath);
    bool processWav(const std::string& wavPath, const std::string& processedPath,
                    const AudioConvertSettings& settings,
                    ProgressCallback progressCallback);
    bool encodeToFormat(const std::string& processedPath, const std::string& outputPath,
                        const AudioConvertSettings& settings,
                        ProgressCallback progressCallback);
    
    bool runExternalTool(const std::string& toolPath, const std::string& args,
                         ProgressCallback progressCallback);
    
    std::string detectInputFormat(const std::string& filePath) const;
    bool validateSettings(const AudioConvertSettings& settings) const;
    
    std::thread convertThread_;
    std::mutex mutex_;
    std::atomic<bool> converting_;
    std::atomic<bool> cancelled_;
    std::atomic<float> progress_;
    
    std::unordered_map<std::string, std::string> toolPaths_;
    std::vector<Preset> presets_;
    
    void initDefaultPresets();
    void initDefaultToolPaths();
};

}
}
