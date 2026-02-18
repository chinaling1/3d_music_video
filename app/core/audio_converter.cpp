/**
 * @file audio_converter.cpp
 * @brief 音频格式转换器实现
 */

#include "audio_converter.h"
#include "../../src/core/logger.h"
#include "../../src/audio/audio_buffer.h"
#include <filesystem>
#include <sstream>
#include <cstdlib>

namespace fs = std::filesystem;
namespace v3d {
namespace app {

AudioConverter::AudioConverter()
    : converting_(false)
    , cancelled_(false)
    , progress_(0.0f)
{
    initDefaultPresets();
    initDefaultToolPaths();
}

AudioConverter::~AudioConverter() {
    cancel();
    if (convertThread_.joinable()) {
        convertThread_.join();
    }
}

AudioConverter& AudioConverter::getInstance() {
    static AudioConverter instance;
    return instance;
}

void AudioConverter::initDefaultPresets() {
    presets_ = {
        {"高质量AAC", "适合一般用途的高质量AAC编码", 
         {AudioCodec::AAC, AudioSampleRate::SR_48000, AudioBitDepth::Bit16, AudioChannels::Stereo, 256}},
        {"标准MP3", "兼容性最好的MP3格式",
         {AudioCodec::MP3, AudioSampleRate::SR_44100, AudioBitDepth::Bit16, AudioChannels::Stereo, 192}},
        {"无损FLAC", "无损压缩，适合存档",
         {AudioCodec::FLAC, AudioSampleRate::SR_48000, AudioBitDepth::Bit24, AudioChannels::Stereo, 0, 8}},
        {"语音优化", "针对语音内容优化",
         {AudioCodec::Opus, AudioSampleRate::SR_48000, AudioBitDepth::Bit16, AudioChannels::Mono, 64, 5, true}},
        {"视频配乐", "适合作为视频背景音乐",
         {AudioCodec::AAC, AudioSampleRate::SR_48000, AudioBitDepth::Bit16, AudioChannels::Stereo, 192, 5, true, 0.9f, true}}
    };
}

void AudioConverter::initDefaultToolPaths() {
#ifdef _WIN32
    toolPaths_["ffmpeg"] = "ffmpeg.exe";
    toolPaths_["sox"] = "sox.exe";
#else
    toolPaths_["ffmpeg"] = "ffmpeg";
    toolPaths_["sox"] = "sox";
#endif
}

bool AudioConverter::convert(const std::string& inputPath, const std::string& outputPath,
                              const AudioConvertSettings& settings) {
    if (converting_) return false;
    
    converting_ = true;
    cancelled_ = false;
    progress_ = 0.0f;
    
    bool result = false;
    
    std::string tempWav = fs::temp_directory_path().string() + "/temp_input.wav";
    std::string tempProcessed = fs::temp_directory_path().string() + "/temp_processed.wav";
    
    if (convertToWav(inputPath, tempWav)) {
        progress_ = 0.3f;
        
        if (!cancelled_ && processWav(tempWav, tempProcessed, settings, nullptr)) {
            progress_ = 0.6f;
            
            if (!cancelled_ && encodeToFormat(tempProcessed, outputPath, settings, nullptr)) {
                progress_ = 1.0f;
                result = true;
            }
        }
    }
    
    fs::remove(tempWav);
    fs::remove(tempProcessed);
    
    converting_ = false;
    return result;
}

void AudioConverter::convertAsync(const std::string& inputPath, const std::string& outputPath,
                                   const AudioConvertSettings& settings,
                                   ProgressCallback progressCallback,
                                   CompletionCallback completionCallback) {
    if (converting_) {
        if (completionCallback) {
            completionCallback(false, "Already converting");
        }
        return;
    }
    
    if (convertThread_.joinable()) {
        convertThread_.join();
    }
    
    convertThread_ = std::thread(&AudioConverter::convertThread, this,
                                  inputPath, outputPath, settings,
                                  progressCallback, completionCallback);
}

void AudioConverter::convertThread(const std::string& inputPath, const std::string& outputPath,
                                    AudioConvertSettings settings,
                                    ProgressCallback progressCallback,
                                    CompletionCallback completionCallback) {
    converting_ = true;
    cancelled_ = false;
    progress_ = 0.0f;
    
    bool success = false;
    std::string errorMessage;
    
    try {
        std::string tempWav = fs::temp_directory_path().string() + "/temp_input_" + std::to_string(std::time(nullptr)) + ".wav";
        std::string tempProcessed = fs::temp_directory_path().string() + "/temp_processed_" + std::to_string(std::time(nullptr)) + ".wav";
        
        if (progressCallback) {
            AudioConvertProgress p;
            p.progress = 0.0f;
            p.currentStage = "正在解码输入文件...";
            p.currentPass = 1;
            p.totalPasses = 3;
            progressCallback(p);
        }
        
        if (!convertToWav(inputPath, tempWav)) {
            errorMessage = "无法解码输入文件";
        } else if (!cancelled_) {
            progress_ = 0.3f;
            
            if (progressCallback) {
                AudioConvertProgress p;
                p.progress = 0.3f;
                p.currentStage = "正在处理音频...";
                p.currentPass = 2;
                p.totalPasses = 3;
                progressCallback(p);
            }
            
            if (!processWav(tempWav, tempProcessed, settings, progressCallback)) {
                errorMessage = "音频处理失败";
            } else if (!cancelled_) {
                progress_ = 0.6f;
                
                if (progressCallback) {
                    AudioConvertProgress p;
                    p.progress = 0.6f;
                    p.currentStage = "正在编码输出文件...";
                    p.currentPass = 3;
                    p.totalPasses = 3;
                    progressCallback(p);
                }
                
                if (!encodeToFormat(tempProcessed, outputPath, settings, progressCallback)) {
                    errorMessage = "编码失败";
                } else {
                    progress_ = 1.0f;
                    success = true;
                }
            }
        }
        
        fs::remove(tempWav);
        fs::remove(tempProcessed);
        
    } catch (const std::exception& e) {
        errorMessage = e.what();
    }
    
    if (cancelled_) {
        errorMessage = "已取消";
    }
    
    if (progressCallback) {
        AudioConvertProgress p;
        p.progress = success ? 1.0f : progress_.load();
        p.currentStage = success ? "完成" : "失败";
        p.currentPass = 3;
        p.totalPasses = 3;
        progressCallback(p);
    }
    
    converting_ = false;
    
    if (completionCallback) {
        completionCallback(success, errorMessage);
    }
}

void AudioConverter::cancel() {
    cancelled_ = true;
}

bool AudioConverter::isConverting() const {
    return converting_;
}

float AudioConverter::getProgress() const {
    return progress_;
}

bool AudioConverter::convertToWav(const std::string& inputPath, const std::string& wavPath) {
    std::string ffmpeg = getToolPath("ffmpeg");
    std::string args = " -i \"" + inputPath + "\" -y \"" + wavPath + "\"";
    
    return runExternalTool(ffmpeg, args, nullptr);
}

bool AudioConverter::processWav(const std::string& wavPath, const std::string& processedPath,
                                 const AudioConvertSettings& settings,
                                 ProgressCallback progressCallback) {
    v3d::audio::AudioBuffer buffer;
    
    if (!buffer.loadFromFile(wavPath)) {
        return false;
    }
    
    if (settings.useTimeRange && settings.endTime > settings.startTime) {
        int startSample = static_cast<int>(settings.startTime * buffer.getSpec().sampleRate);
        int endSample = static_cast<int>(settings.endTime * buffer.getSpec().sampleRate);
    }
    
    if (settings.normalize) {
        buffer.applyGain(settings.normalizeTarget);
    }
    
    if (settings.fadeIn && settings.fadeInDuration > 0) {
        int fadeSamples = static_cast<int>(settings.fadeInDuration * buffer.getSpec().sampleRate);
        buffer.applyFadeIn(fadeSamples);
    }
    
    if (settings.fadeOut && settings.fadeOutDuration > 0) {
        int fadeSamples = static_cast<int>(settings.fadeOutDuration * buffer.getSpec().sampleRate);
        buffer.applyFadeOut(fadeSamples);
    }
    
    int targetSampleRate = static_cast<int>(settings.sampleRate);
    if (buffer.getSpec().sampleRate != targetSampleRate) {
        buffer.resample(targetSampleRate);
    }
    
    if (!buffer.saveToFile(processedPath)) {
        return false;
    }
    
    return true;
}

bool AudioConverter::encodeToFormat(const std::string& processedPath, const std::string& outputPath,
                                     const AudioConvertSettings& settings,
                                     ProgressCallback progressCallback) {
    std::string ffmpeg = getToolPath("ffmpeg");
    std::string args;
    
    args = " -i \"" + processedPath + "\"";
    
    switch (settings.codec) {
        case AudioCodec::MP3:
            args += " -codec:a libmp3lame -b:a " + std::to_string(settings.bitrate) + "k";
            break;
        case AudioCodec::AAC:
            args += " -codec:a aac -b:a " + std::to_string(settings.bitrate) + "k";
            break;
        case AudioCodec::Vorbis:
            args += " -codec:a libvorbis -q:a " + std::to_string(settings.quality);
            break;
        case AudioCodec::FLAC:
            args += " -codec:a flac -compression_level " + std::to_string(settings.quality);
            break;
        case AudioCodec::Opus:
            args += " -codec:a libopus -b:a " + std::to_string(settings.bitrate) + "k";
            break;
        case AudioCodec::WAV:
        case AudioCodec::PCM:
            args += " -codec:a pcm_s16le";
            break;
    }
    
    args += " -y \"" + outputPath + "\"";
    
    return runExternalTool(ffmpeg, args, progressCallback);
}

bool AudioConverter::runExternalTool(const std::string& toolPath, const std::string& args,
                                      ProgressCallback progressCallback) {
    std::string cmd = toolPath + args + " 2>&1";
    
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return false;
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        if (cancelled_) {
            _pclose(pipe);
            return false;
        }
    }
    
    int result = _pclose(pipe);
    return result == 0;
}

std::vector<std::string> AudioConverter::getSupportedInputFormats() const {
    return {".wav", ".mp3", ".ogg", ".flac", ".aac", ".m4a", ".wma", ".ape", ".opus"};
}

std::vector<std::string> AudioConverter::getSupportedOutputFormats() const {
    return {".wav", ".mp3", ".aac", ".ogg", ".flac", ".opus"};
}

bool AudioConverter::getAudioInfo(const std::string& filePath,
                                   int& sampleRate, int& channels, int& bitDepth,
                                   float& duration) {
    v3d::audio::AudioBuffer buffer;
    if (!buffer.loadFromFile(filePath)) {
        return false;
    }
    
    sampleRate = buffer.getSpec().sampleRate;
    channels = buffer.getSpec().getChannelCount();
    bitDepth = buffer.getSpec().getBytesPerSample() * 8;
    duration = static_cast<float>(buffer.getFrameCount()) / sampleRate;
    
    return true;
}

std::string AudioConverter::codecToString(AudioCodec codec) {
    switch (codec) {
        case AudioCodec::PCM: return "PCM";
        case AudioCodec::MP3: return "MP3";
        case AudioCodec::AAC: return "AAC";
        case AudioCodec::Vorbis: return "Vorbis";
        case AudioCodec::FLAC: return "FLAC";
        case AudioCodec::Opus: return "Opus";
        case AudioCodec::WAV: return "WAV";
        default: return "Unknown";
    }
}

AudioCodec AudioConverter::stringToCodec(const std::string& str) {
    if (str == "MP3") return AudioCodec::MP3;
    if (str == "AAC") return AudioCodec::AAC;
    if (str == "Vorbis" || str == "OGG") return AudioCodec::Vorbis;
    if (str == "FLAC") return AudioCodec::FLAC;
    if (str == "Opus") return AudioCodec::Opus;
    if (str == "WAV") return AudioCodec::WAV;
    return AudioCodec::PCM;
}

std::string AudioConverter::getCodecExtension(AudioCodec codec) {
    switch (codec) {
        case AudioCodec::MP3: return ".mp3";
        case AudioCodec::AAC: return ".m4a";
        case AudioCodec::Vorbis: return ".ogg";
        case AudioCodec::FLAC: return ".flac";
        case AudioCodec::Opus: return ".opus";
        case AudioCodec::WAV:
        case AudioCodec::PCM: return ".wav";
        default: return ".wav";
    }
}

void AudioConverter::setToolPath(const std::string& toolName, const std::string& path) {
    toolPaths_[toolName] = path;
}

std::string AudioConverter::getToolPath(const std::string& toolName) const {
    auto it = toolPaths_.find(toolName);
    if (it != toolPaths_.end()) return it->second;
    return toolName;
}

std::vector<AudioConverter::Preset> AudioConverter::getPresets() const {
    return presets_;
}

void AudioConverter::addPreset(const Preset& preset) {
    presets_.push_back(preset);
}

void AudioConverter::removePreset(const std::string& name) {
    presets_.erase(
        std::remove_if(presets_.begin(), presets_.end(),
            [&name](const Preset& p) { return p.name == name; }),
        presets_.end()
    );
}

bool AudioConverter::batchConvert(const std::vector<std::string>& inputPaths,
                                   const std::string& outputDirectory,
                                   const AudioConvertSettings& settings,
                                   ProgressCallback progressCallback) {
    int total = static_cast<int>(inputPaths.size());
    int current = 0;
    
    for (const auto& inputPath : inputPaths) {
        if (cancelled_) return false;
        
        fs::path p(inputPath);
        std::string outputPath = outputDirectory + "/" + p.stem().string() + 
                                  getCodecExtension(settings.codec);
        
        if (progressCallback) {
            AudioConvertProgress prog;
            prog.progress = static_cast<float>(current) / total;
            prog.currentStage = "正在处理: " + p.filename().string();
            prog.currentPass = current + 1;
            prog.totalPasses = total;
            progressCallback(prog);
        }
        
        if (!convert(inputPath, outputPath, settings)) {
            return false;
        }
        
        current++;
    }
    
    return true;
}

}
}
