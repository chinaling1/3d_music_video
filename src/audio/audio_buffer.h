/**
 * @file audio_buffer.h
 * @brief 音频缓冲区 - 提供音频数据的存储和处理功能
 * 
 * 该模块实现了完整的音频数据处理系统，包括：
 * - AudioFormat: 音频格式枚举
 * - AudioChannelLayout: 声道布局枚举
 * - AudioSpec: 音频规格结构
 * - AudioBuffer: 音频缓冲区类
 * - AudioStream: 音频流基类
 * - AudioFileStream: 文件音频流
 * - AudioGenerator: 音频生成器
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>

namespace v3d {
namespace audio {

/**
 * @enum AudioFormat
 * @brief 音频采样格式枚举
 * 
 * 定义了支持的音频采样格式。
 */
enum class AudioFormat {
    Unknown,  ///< 未知格式
    PCM8,     ///< 8位PCM
    PCM16,    ///< 16位PCM
    PCM24,    ///< 24位PCM
    PCM32,    ///< 32位PCM
    Float32,  ///< 32位浮点
    Float64   ///< 64位浮点
};

/**
 * @enum AudioChannelLayout
 * @brief 声道布局枚举
 * 
 * 定义了支持的声道配置。
 */
enum class AudioChannelLayout {
    Mono,       ///< 单声道
    Stereo,     ///< 立体声
    Quad,       ///< 四声道
    Surround51, ///< 5.1环绕声
    Surround71  ///< 7.1环绕声
};

/**
 * @struct AudioSpec
 * @brief 音频规格结构
 * 
 * 描述音频数据的格式参数。
 */
struct AudioSpec {
    int sampleRate;              ///< 采样率（Hz）
    AudioFormat format;          ///< 采样格式
    AudioChannelLayout channels; ///< 声道布局
    int frameSize;               ///< 帧大小

    /**
     * @brief 获取每样本字节数
     * @return 字节数
     */
    int getBytesPerSample() const;
    
    /**
     * @brief 获取每帧字节数
     * @return 字节数
     */
    int getBytesPerFrame() const;
    
    /**
     * @brief 获取声道数
     * @return 声道数量
     */
    int getChannelCount() const;
};

/**
 * @class AudioBuffer
 * @brief 音频缓冲区 - 存储和处理音频数据
 * 
 * 提供音频数据的存储、处理和转换功能。
 * 支持多种格式和声道布局。
 * 
 * @example
 * @code
 * // 创建音频缓冲区
 * AudioSpec spec;
 * spec.sampleRate = 44100;
 * spec.format = AudioFormat::Float32;
 * spec.channels = AudioChannelLayout::Stereo;
 * 
 * AudioBuffer buffer(spec, 44100);  // 1秒音频
 * 
 * // 填充数据
 * buffer.fill(0.0f);
 * 
 * // 应用效果
 * buffer.applyGain(0.8f);
 * buffer.applyFadeIn(1000);
 * 
 * // 格式转换
 * buffer.resample(48000);
 * buffer.convertChannels(AudioChannelLayout::Mono);
 * 
 * // 保存到文件
 * buffer.saveToFile("output.wav");
 * @endcode
 */
class AudioBuffer {
public:
    AudioBuffer();
    
    /**
     * @brief 构造指定规格的缓冲区
     * @param spec 音频规格
     * @param frameCount 帧数
     */
    AudioBuffer(const AudioSpec& spec, size_t frameCount);
    
    ~AudioBuffer();

    /**
     * @brief 分配缓冲区内存
     * @param spec 音频规格
     * @param frameCount 帧数
     * @return 成功返回true
     */
    bool allocate(const AudioSpec& spec, size_t frameCount);
    
    /**
     * @brief 释放缓冲区内存
     */
    void deallocate();

    /**
     * @brief 获取音频规格
     * @return AudioSpec引用
     */
    const AudioSpec& getSpec() const;
    
    /**
     * @brief 获取帧数
     * @return 帧数
     */
    size_t getFrameCount() const;
    
    /**
     * @brief 获取样本数
     * @return 样本总数（帧数 * 声道数）
     */
    size_t getSampleCount() const;
    
    /**
     * @brief 获取数据大小
     * @return 字节数
     */
    size_t getSizeInBytes() const;

    /**
     * @brief 获取数据指针
     * @return 数据指针
     */
    void* getData();
    
    /**
     * @brief 获取数据指针（只读）
     * @return 数据指针
     */
    const void* getData() const;

    /**
     * @brief 清空数据
     */
    void clear();
    
    /**
     * @brief 填充指定值
     * @param value 填充值
     */
    void fill(float value);

    /**
     * @brief 从另一个缓冲区复制
     * @param other 源缓冲区
     */
    void copyFrom(const AudioBuffer& other);
    
    /**
     * @brief 从另一个缓冲区混合
     * @param other 源缓冲区
     * @param gain 混合增益
     */
    void mixFrom(const AudioBuffer& other, float gain = 1.0f);

    /**
     * @brief 重采样
     * @param newSampleRate 新采样率
     */
    void resample(int newSampleRate);
    
    /**
     * @brief 转换格式
     * @param newFormat 新格式
     */
    void convertFormat(AudioFormat newFormat);
    
    /**
     * @brief 转换声道布局
     * @param newLayout 新声道布局
     */
    void convertChannels(AudioChannelLayout newLayout);

    /**
     * @brief 应用增益
     * @param gain 增益值
     */
    void applyGain(float gain);
    
    /**
     * @brief 应用淡入效果
     * @param frameCount 淡入帧数
     */
    void applyFadeIn(size_t frameCount);
    
    /**
     * @brief 应用淡出效果
     * @param frameCount 淡出帧数
     */
    void applyFadeOut(size_t frameCount);

    /**
     * @brief 保存到文件
     * @param filePath 文件路径
     * @return 成功返回true
     */
    bool saveToFile(const std::string& filePath) const;
    
    /**
     * @brief 从文件加载
     * @param filePath 文件路径
     * @return 成功返回true
     */
    bool loadFromFile(const std::string& filePath);

    /**
     * @brief 克隆缓冲区
     * @return 新缓冲区
     */
    AudioBuffer clone() const;

private:
    AudioSpec spec_;              ///< 音频规格
    std::vector<uint8_t> data_;   ///< 数据存储
    size_t frameCount_;           ///< 帧数
};

/**
 * @class AudioStream
 * @brief 音频流基类
 * 
 * 抽象基类，定义音频流的接口。
 */
class AudioStream {
public:
    AudioStream();
    virtual ~AudioStream();

    /**
     * @brief 打开音频流
     * @param filePath 文件路径
     * @return 成功返回true
     */
    virtual bool open(const std::string& filePath) = 0;
    
    /**
     * @brief 关闭音频流
     */
    virtual void close() = 0;

    /**
     * @brief 读取音频数据
     * @param buffer 目标缓冲区
     * @param frames 要读取的帧数
     * @return 实际读取的帧数
     */
    virtual size_t read(void* buffer, size_t frames) = 0;
    
    /**
     * @brief 定位到指定帧
     * @param frame 帧位置
     * @return 成功返回true
     */
    virtual bool seek(size_t frame) = 0;
    
    /**
     * @brief 获取当前位置
     * @return 当前帧位置
     */
    virtual size_t tell() const = 0;
    
    /**
     * @brief 获取总帧数
     * @return 总帧数
     */
    virtual size_t getTotalFrames() const = 0;

    /**
     * @brief 获取音频规格
     * @return AudioSpec引用
     */
    const AudioSpec& getSpec() const;
    
    /**
     * @brief 检查是否已打开
     * @return 已打开返回true
     */
    bool isOpen() const;

protected:
    AudioSpec spec_;  ///< 音频规格
    bool open_;       ///< 打开状态
};

/**
 * @class AudioFileStream
 * @brief 文件音频流 - 从文件读取音频数据
 * 
 * 支持从音频文件流式读取数据。
 * 
 * @example
 * @code
 * AudioFileStream stream;
 * stream.open("music.wav");
 * 
 * std::vector<float> buffer(1024 * 2);  // 立体声
 * size_t framesRead = stream.read(buffer.data(), 1024);
 * 
 * stream.close();
 * @endcode
 */
class AudioFileStream : public AudioStream {
public:
    AudioFileStream();
    ~AudioFileStream() override;

    bool open(const std::string& filePath) override;
    void close() override;

    size_t read(void* buffer, size_t frames) override;
    bool seek(size_t frame) override;
    size_t tell() const override;
    size_t getTotalFrames() const override;

    /**
     * @brief 检查是否循环播放
     * @return 循环返回true
     */
    bool isLooping() const;
    
    /**
     * @brief 设置循环播放
     * @param looping 是否循环
     */
    void setLooping(bool looping);

private:
    std::vector<uint8_t> fileData_;  ///< 文件数据
    size_t currentFrame_;            ///< 当前帧位置
    bool looping_;                   ///< 循环标志
};

/**
 * @class AudioGenerator
 * @brief 音频生成器 - 生成程序化音频波形
 * 
 * 支持生成各种基本波形。
 * 
 * @example
 * @code
 * AudioGenerator generator;
 * generator.setWaveformType(AudioGenerator::Sine);
 * generator.setFrequency(440.0f);  // A4音符
 * generator.setAmplitude(0.5f);
 * 
 * std::vector<float> buffer(1024);
 * generator.read(buffer.data(), 1024);
 * @endcode
 */
class AudioGenerator : public AudioStream {
public:
    AudioGenerator();
    ~AudioGenerator() override;

    bool open(const std::string& filePath) override;
    void close() override;

    size_t read(void* buffer, size_t frames) override;
    bool seek(size_t frame) override;
    size_t tell() const override;
    size_t getTotalFrames() const override;

    /**
     * @brief 设置波形类型
     * @param type 波形类型
     */
    void setWaveformType(int type);
    
    /**
     * @brief 设置频率
     * @param frequency 频率（Hz）
     */
    void setFrequency(float frequency);
    
    /**
     * @brief 设置振幅
     * @param amplitude 振幅 [0, 1]
     */
    void setAmplitude(float amplitude);

    /**
     * @enum WaveformType
     * @brief 波形类型枚举
     */
    enum WaveformType {
        Sine,     ///< 正弦波
        Square,   ///< 方波
        Sawtooth, ///< 锯齿波
        Triangle, ///< 三角波
        Noise     ///< 噪声
    };

private:
    int waveformType_;    ///< 波形类型
    float frequency_;     ///< 频率
    float amplitude_;     ///< 振幅
    size_t currentFrame_; ///< 当前帧位置
};

}
}
