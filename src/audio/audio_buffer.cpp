#include "audio_buffer.h"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace v3d {
namespace audio {

int AudioSpec::getBytesPerSample() const {
    switch (format) {
        case AudioFormat::PCM8: return 1;
        case AudioFormat::PCM16: return 2;
        case AudioFormat::PCM24: return 3;
        case AudioFormat::PCM32: return 4;
        case AudioFormat::Float32: return 4;
        case AudioFormat::Float64: return 8;
        default: return 0;
    }
}

int AudioSpec::getBytesPerFrame() const {
    return getBytesPerSample() * getChannelCount();
}

int AudioSpec::getChannelCount() const {
    switch (channels) {
        case AudioChannelLayout::Mono: return 1;
        case AudioChannelLayout::Stereo: return 2;
        case AudioChannelLayout::Quad: return 4;
        case AudioChannelLayout::Surround51: return 6;
        case AudioChannelLayout::Surround71: return 8;
        default: return 0;
    }
}

AudioBuffer::AudioBuffer()
    : frameCount_(0) {
}

AudioBuffer::AudioBuffer(const AudioSpec& spec, size_t frameCount)
    : spec_(spec)
    , frameCount_(frameCount) {
    allocate(spec, frameCount);
}

AudioBuffer::~AudioBuffer() {
    deallocate();
}

bool AudioBuffer::allocate(const AudioSpec& spec, size_t frameCount) {
    spec_ = spec;
    frameCount_ = frameCount;

    size_t size = spec.getBytesPerFrame() * frameCount;
    data_.resize(size);

    return true;
}

void AudioBuffer::deallocate() {
    data_.clear();
    frameCount_ = 0;
}

const AudioSpec& AudioBuffer::getSpec() const {
    return spec_;
}

size_t AudioBuffer::getFrameCount() const {
    return frameCount_;
}

size_t AudioBuffer::getSampleCount() const {
    return frameCount_ * spec_.getChannelCount();
}

size_t AudioBuffer::getSizeInBytes() const {
    return data_.size();
}

void* AudioBuffer::getData() {
    return data_.data();
}

const void* AudioBuffer::getData() const {
    return data_.data();
}

void AudioBuffer::clear() {
    std::fill(data_.begin(), data_.end(), 0);
}

void AudioBuffer::fill(float value) {
    if (spec_.format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(data_.data());
        size_t sampleCount = getSampleCount();
        for (size_t i = 0; i < sampleCount; ++i) {
            samples[i] = value;
        }
    }
}

void AudioBuffer::copyFrom(const AudioBuffer& other) {
    if (other.getSpec().sampleRate != spec_.sampleRate ||
        other.getSpec().format != spec_.format ||
        other.getSpec().channels != spec_.channels) {
        return;
    }

    size_t copySize = std::min(getSizeInBytes(), other.getSizeInBytes());
    std::memcpy(data_.data(), other.getData(), copySize);
}

void AudioBuffer::mixFrom(const AudioBuffer& other, float gain) {
    if (other.getSpec().sampleRate != spec_.sampleRate ||
        other.getSpec().format != spec_.format ||
        other.getSpec().channels != spec_.channels) {
        return;
    }

    if (spec_.format == AudioFormat::Float32) {
        float* destSamples = reinterpret_cast<float*>(data_.data());
        const float* srcSamples = reinterpret_cast<const float*>(other.getData());

        size_t sampleCount = std::min(getSampleCount(), other.getSampleCount());
        for (size_t i = 0; i < sampleCount; ++i) {
            destSamples[i] += srcSamples[i] * gain;
        }
    } else if (spec_.format == AudioFormat::PCM16) {
        int16_t* destSamples = reinterpret_cast<int16_t*>(data_.data());
        const int16_t* srcSamples = reinterpret_cast<const int16_t*>(other.getData());

        size_t sampleCount = std::min(getSampleCount(), other.getSampleCount());
        for (size_t i = 0; i < sampleCount; ++i) {
            int32_t mixed = static_cast<int32_t>(destSamples[i]) +
                           static_cast<int32_t>(srcSamples[i] * gain);
            destSamples[i] = static_cast<int16_t>(std::clamp(mixed, -32768, 32767));
        }
    }
}

void AudioBuffer::resample(int newSampleRate) {
    if (newSampleRate == spec_.sampleRate || frameCount_ == 0) {
        return;
    }

    float ratio = static_cast<float>(newSampleRate) / static_cast<float>(spec_.sampleRate);
    size_t newFrameCount = static_cast<size_t>(frameCount_ * ratio);

    AudioBuffer newBuffer;
    newBuffer.allocate(spec_, newFrameCount);

    if (spec_.format == AudioFormat::Float32) {
        const float* srcSamples = reinterpret_cast<const float*>(data_.data());
        float* destSamples = reinterpret_cast<float*>(newBuffer.getData());

        for (size_t i = 0; i < newFrameCount; ++i) {
            float srcPos = static_cast<float>(i) / ratio;
            size_t srcIndex = static_cast<size_t>(srcPos);
            float alpha = srcPos - srcPos;

            if (srcIndex + 1 < frameCount_) {
                destSamples[i] = srcSamples[srcIndex] * (1.0f - alpha) +
                                 srcSamples[srcIndex + 1] * alpha;
            } else {
                destSamples[i] = srcSamples[srcIndex];
            }
        }
    }

    *this = newBuffer;
    spec_.sampleRate = newSampleRate;
}

void AudioBuffer::convertFormat(AudioFormat newFormat) {
    if (newFormat == spec_.format) {
        return;
    }

    AudioBuffer newBuffer;
    AudioSpec newSpec = spec_;
    newSpec.format = newFormat;
    newBuffer.allocate(newSpec, frameCount_);

    if (spec_.format == AudioFormat::PCM16 && newFormat == AudioFormat::Float32) {
        const int16_t* srcSamples = reinterpret_cast<const int16_t*>(data_.data());
        float* destSamples = reinterpret_cast<float*>(newBuffer.getData());

        size_t sampleCount = getSampleCount();
        for (size_t i = 0; i < sampleCount; ++i) {
            destSamples[i] = static_cast<float>(srcSamples[i]) / 32768.0f;
        }
    } else if (spec_.format == AudioFormat::Float32 && newFormat == AudioFormat::PCM16) {
        const float* srcSamples = reinterpret_cast<const float*>(data_.data());
        int16_t* destSamples = reinterpret_cast<int16_t*>(newBuffer.getData());

        size_t sampleCount = getSampleCount();
        for (size_t i = 0; i < sampleCount; ++i) {
            float sample = std::clamp(srcSamples[i] * 32768.0f, -32768.0f, 32767.0f);
            destSamples[i] = static_cast<int16_t>(sample);
        }
    }

    *this = newBuffer;
}

void AudioBuffer::convertChannels(AudioChannelLayout newLayout) {
    if (newLayout == spec_.channels) {
        return;
    }

    AudioBuffer newBuffer;
    AudioSpec newSpec = spec_;
    newSpec.channels = newLayout;
    newBuffer.allocate(newSpec, frameCount_);

    if (spec_.format == AudioFormat::Float32) {
        const float* srcSamples = reinterpret_cast<const float*>(data_.data());
        float* destSamples = reinterpret_cast<float*>(newBuffer.getData());

        int srcChannels = spec_.getChannelCount();
        int destChannels = newSpec.getChannelCount();

        for (size_t frame = 0; frame < frameCount_; ++frame) {
            for (int ch = 0; ch < destChannels; ++ch) {
                if (ch < srcChannels) {
                    destSamples[frame * destChannels + ch] = srcSamples[frame * srcChannels + ch];
                } else {
                    destSamples[frame * destChannels + ch] = 0.0f;
                }
            }
        }
    }

    *this = newBuffer;
}

void AudioBuffer::applyGain(float gain) {
    if (spec_.format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(data_.data());
        size_t sampleCount = getSampleCount();
        for (size_t i = 0; i < sampleCount; ++i) {
            samples[i] *= gain;
        }
    } else if (spec_.format == AudioFormat::PCM16) {
        int16_t* samples = reinterpret_cast<int16_t*>(data_.data());
        size_t sampleCount = getSampleCount();
        for (size_t i = 0; i < sampleCount; ++i) {
            int32_t sample = static_cast<int32_t>(samples[i] * gain);
            samples[i] = static_cast<int16_t>(std::clamp(sample, -32768, 32767));
        }
    }
}

void AudioBuffer::applyFadeIn(size_t frameCount) {
    if (spec_.format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(data_.data());
        size_t fadeFrames = std::min(frameCount, frameCount_);
        int channels = spec_.getChannelCount();

        for (size_t frame = 0; frame < fadeFrames; ++frame) {
            float gain = static_cast<float>(frame) / static_cast<float>(fadeFrames);
            for (int ch = 0; ch < channels; ++ch) {
                samples[frame * channels + ch] *= gain;
            }
        }
    }
}

void AudioBuffer::applyFadeOut(size_t frameCount) {
    if (spec_.format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(data_.data());
        size_t fadeFrames = std::min(frameCount, frameCount_);
        int channels = spec_.getChannelCount();

        for (size_t frame = 0; frame < fadeFrames; ++frame) {
            size_t startFrame = frameCount_ - fadeFrames + frame;
            float gain = 1.0f - static_cast<float>(frame) / static_cast<float>(fadeFrames);
            for (int ch = 0; ch < channels; ++ch) {
                samples[startFrame * channels + ch] *= gain;
            }
        }
    }
}

bool AudioBuffer::saveToFile(const std::string& filePath) const {
    return false;
}

bool AudioBuffer::loadFromFile(const std::string& filePath) {
    return false;
}

AudioBuffer AudioBuffer::clone() const {
    AudioBuffer buffer;
    buffer.allocate(spec_, frameCount_);
    std::memcpy(buffer.getData(), data_.data(), data_.size());
    return buffer;
}

AudioStream::AudioStream()
    : open_(false) {
}

AudioStream::~AudioStream() {
}

const AudioSpec& AudioStream::getSpec() const {
    return spec_;
}

bool AudioStream::isOpen() const {
    return open_;
}

AudioFileStream::AudioFileStream()
    : AudioStream()
    , currentFrame_(0)
    , looping_(false) {
}

AudioFileStream::~AudioFileStream() {
    close();
}

bool AudioFileStream::open(const std::string& filePath) {
    close();

    spec_.sampleRate = 44100;
    spec_.format = AudioFormat::Float32;
    spec_.channels = AudioChannelLayout::Stereo;
    spec_.frameSize = spec_.getBytesPerFrame();

    open_ = true;
    currentFrame_ = 0;

    return true;
}

void AudioFileStream::close() {
    fileData_.clear();
    open_ = false;
    currentFrame_ = 0;
}

size_t AudioFileStream::read(void* buffer, size_t frames) {
    if (!open_ || frames == 0) {
        return 0;
    }

    size_t framesToRead = std::min(frames, fileData_.size() / spec_.getBytesPerFrame() - currentFrame_);

    if (framesToRead == 0 && looping_) {
        currentFrame_ = 0;
        framesToRead = std::min(frames, fileData_.size() / spec_.getBytesPerFrame());
    }

    if (framesToRead == 0) {
        return 0;
    }

    size_t bytesToRead = framesToRead * spec_.getBytesPerFrame();
    std::memcpy(buffer, fileData_.data() + currentFrame_ * spec_.getBytesPerFrame(), bytesToRead);
    currentFrame_ += framesToRead;

    return framesToRead;
}

bool AudioFileStream::seek(size_t frame) {
    if (!open_) {
        return false;
    }

    size_t totalFrames = fileData_.size() / spec_.getBytesPerFrame();
    currentFrame_ = std::min(frame, totalFrames);

    return true;
}

size_t AudioFileStream::tell() const {
    return currentFrame_;
}

size_t AudioFileStream::getTotalFrames() const {
    return fileData_.size() / spec_.getBytesPerFrame();
}

bool AudioFileStream::isLooping() const {
    return looping_;
}

void AudioFileStream::setLooping(bool looping) {
    looping_ = looping;
}

AudioGenerator::AudioGenerator()
    : AudioStream()
    , waveformType_(Sine)
    , frequency_(440.0f)
    , amplitude_(1.0f)
    , currentFrame_(0) {
}

AudioGenerator::~AudioGenerator() {
}

bool AudioGenerator::open(const std::string& filePath) {
    spec_.sampleRate = 44100;
    spec_.format = AudioFormat::Float32;
    spec_.channels = AudioChannelLayout::Mono;
    spec_.frameSize = spec_.getBytesPerFrame();

    open_ = true;
    currentFrame_ = 0;

    return true;
}

void AudioGenerator::close() {
    open_ = false;
    currentFrame_ = 0;
}

size_t AudioGenerator::read(void* buffer, size_t frames) {
    if (!open_ || frames == 0) {
        return 0;
    }

    float* samples = reinterpret_cast<float*>(buffer);
    float sampleRate = static_cast<float>(spec_.sampleRate);
    float angularFrequency = 2.0f * 3.14159265359f * frequency_ / sampleRate;

    for (size_t i = 0; i < frames; ++i) {
        float t = static_cast<float>(currentFrame_ + i);
        float sample = 0.0f;

        switch (waveformType_) {
            case Sine:
                sample = std::sin(angularFrequency * t);
                break;
            case Square:
                sample = std::sin(angularFrequency * t) >= 0.0f ? 1.0f : -1.0f;
                break;
            case Sawtooth:
                sample = 2.0f * (t * frequency_ / sampleRate - std::floor(0.5f + t * frequency_ / sampleRate));
                break;
            case Triangle:
                sample = 2.0f * std::abs(2.0f * (t * frequency_ / sampleRate - std::floor(0.5f + t * frequency_ / sampleRate))) - 1.0f;
                break;
            case Noise:
                sample = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
                break;
        }

        samples[i] = sample * amplitude_;
    }

    currentFrame_ += frames;
    return frames;
}

bool AudioGenerator::seek(size_t frame) {
    if (!open_) {
        return false;
    }

    currentFrame_ = frame;
    return true;
}

size_t AudioGenerator::tell() const {
    return currentFrame_;
}

size_t AudioGenerator::getTotalFrames() const {
    return std::numeric_limits<size_t>::max();
}

void AudioGenerator::setWaveformType(int type) {
    waveformType_ = type;
}

void AudioGenerator::setFrequency(float frequency) {
    frequency_ = std::max(0.0f, frequency);
}

void AudioGenerator::setAmplitude(float amplitude) {
    amplitude_ = std::clamp(amplitude, 0.0f, 1.0f);
}

}
}