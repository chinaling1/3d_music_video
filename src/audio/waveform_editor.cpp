#include "waveform_editor.h"
#include "audio_effect.h"
#include <algorithm>
#include <cmath>

namespace v3d {
namespace audio {

WaveformEditor::WaveformEditor()
    : selectionStart_(0.0f)
    , selectionEnd_(0.0f)
    , hasSelection_(false) {
}

WaveformEditor::~WaveformEditor() {
}

void WaveformEditor::setBuffer(std::shared_ptr<AudioBuffer> buffer) {
    buffer_ = buffer;
    clearSelection();
}

std::shared_ptr<AudioBuffer> WaveformEditor::getBuffer() const {
    return buffer_;
}

std::vector<float> WaveformEditor::getWaveformData(int channel, int resolution) const {
    std::vector<float> data(resolution, 0.0f);

    if (!buffer_ || resolution <= 0) {
        return data;
    }

    if (channel < 0 || channel >= buffer_->getSpec().getChannelCount()) {
        return data;
    }

    if (buffer_->getSpec().format != AudioFormat::Float32) {
        return data;
    }

    const float* samples = reinterpret_cast<const float*>(buffer_->getData());
    size_t totalFrames = buffer_->getFrameCount();
    int channels = buffer_->getSpec().getChannelCount();

    size_t samplesPerPixel = totalFrames / resolution;

    for (int i = 0; i < resolution; ++i) {
        float minSample = std::numeric_limits<float>::max();
        float maxSample = std::numeric_limits<float>::lowest();

        size_t startSample = i * samplesPerPixel;
        size_t endSample = std::min(startSample + samplesPerPixel, totalFrames);

        for (size_t j = startSample; j < endSample; ++j) {
            float sample = samples[j * channels + channel];
            minSample = std::min(minSample, sample);
            maxSample = std::max(maxSample, sample);
        }

        data[i] = (maxSample - minSample) * 0.5f;
    }

    return data;
}

void WaveformEditor::cut(float startTime, float endTime) {
    if (!buffer_) {
        return;
    }

    copy(startTime, endTime);
    deleteRange(startTime, endTime);
}

void WaveformEditor::copy(float startTime, float endTime) {
}

void WaveformEditor::paste(float startTime, const AudioBuffer& clipboard) {
}

void WaveformEditor::deleteRange(float startTime, float endTime) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * buffer_->getSpec().sampleRate);
    size_t frameCount = endFrame - startFrame;

    if (startFrame >= buffer_->getFrameCount()) {
        return;
    }

    endFrame = std::min(endFrame, buffer_->getFrameCount());
    frameCount = endFrame - startFrame;

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(buffer_->getData());

        for (size_t i = startFrame; i < buffer_->getFrameCount() - frameCount; ++i) {
            size_t srcIndex = i + frameCount;
            size_t dstIndex = i;

            for (int ch = 0; ch < buffer_->getSpec().getChannelCount(); ++ch) {
                samples[dstIndex * buffer_->getSpec().getChannelCount() + ch] =
                    samples[srcIndex * buffer_->getSpec().getChannelCount() + ch];
            }
        }
    }
}

void WaveformEditor::insertSilence(float startTime, float duration) {
    if (!buffer_) {
        return;
    }

    size_t insertFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t insertFrames = static_cast<size_t>(duration * buffer_->getSpec().sampleRate);

    AudioSpec spec = buffer_->getSpec();
    size_t newFrameCount = buffer_->getFrameCount() + insertFrames;

    AudioBuffer newBuffer;
    newBuffer.allocate(spec, newFrameCount);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        const float* srcSamples = reinterpret_cast<const float*>(buffer_->getData());
        float* dstSamples = reinterpret_cast<float*>(newBuffer.getData());

        size_t channels = spec.getChannelCount();

        for (size_t i = 0; i < insertFrame; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                dstSamples[i * channels + ch] = srcSamples[i * channels + ch];
            }
        }

        for (size_t i = 0; i < insertFrames; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                dstSamples[(insertFrame + i) * channels + ch] = 0.0f;
            }
        }

        for (size_t i = insertFrame; i < buffer_->getFrameCount(); ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                dstSamples[(i + insertFrames) * channels + ch] = srcSamples[i * channels + ch];
            }
        }
    }

    *buffer_ = newBuffer;
}

void WaveformEditor::normalize(float targetLevel) {
    if (!buffer_) {
        return;
    }

    if (buffer_->getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer_->getData());
    size_t sampleCount = buffer_->getSampleCount();

    float maxSample = 0.0f;
    for (size_t i = 0; i < sampleCount; ++i) {
        maxSample = std::max(maxSample, std::abs(samples[i]));
    }

    if (maxSample < 0.0001f) {
        return;
    }

    float gain = targetLevel / maxSample;

    for (size_t i = 0; i < sampleCount; ++i) {
        samples[i] *= gain;
    }
}

void WaveformEditor::amplify(float gain) {
    if (!buffer_) {
        return;
    }

    buffer_->applyGain(gain);
}

void WaveformEditor::fade(float startTime, float endTime, float startLevel, float endLevel) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * buffer_->getSpec().sampleRate);
    size_t frameCount = endFrame - startFrame;

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(buffer_->getData());
        int channels = buffer_->getSpec().getChannelCount();

        for (size_t i = 0; i < frameCount; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(frameCount);
            float level = startLevel + (endLevel - startLevel) * t;

            for (int ch = 0; ch < channels; ++ch) {
                samples[(startFrame + i) * channels + ch] *= level;
            }
        }
    }
}

void WaveformEditor::fadeIn(float startTime, float duration) {
    fade(startTime, startTime + duration, 0.0f, 1.0f);
}

void WaveformEditor::fadeOut(float startTime, float duration) {
    fade(startTime, startTime + duration, 1.0f, 0.0f);
}

void WaveformEditor::reverse(float startTime, float endTime) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * buffer_->getSpec().sampleRate);
    size_t frameCount = endFrame - startFrame;

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(buffer_->getData());
        int channels = buffer_->getSpec().getChannelCount();

        std::vector<float> temp(frameCount * channels);

        for (size_t i = 0; i < frameCount; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                temp[i * channels + ch] = samples[(startFrame + frameCount - 1 - i) * channels + ch];
            }
        }

        for (size_t i = 0; i < frameCount; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                samples[(startFrame + i) * channels + ch] = temp[i * channels + ch];
            }
        }
    }
}

void WaveformEditor::invert(float startTime, float endTime) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * buffer_->getSpec().sampleRate);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(buffer_->getData());
        int channels = buffer_->getSpec().getChannelCount();

        for (size_t i = startFrame; i < endFrame; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                samples[i * channels + ch] = -samples[i * channels + ch];
            }
        }
    }
}

void WaveformEditor::timeStretch(float startTime, float endTime, float ratio) {
    if (!buffer_) {
        return;
    }

    buffer_->resample(static_cast<int>(buffer_->getSpec().sampleRate * ratio));
}

void WaveformEditor::pitchShift(float startTime, float endTime, float semitones) {
}

void WaveformEditor::silence(float startTime, float endTime) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * buffer_->getSpec().sampleRate);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(buffer_->getData());
        int channels = buffer_->getSpec().getChannelCount();

        for (size_t i = startFrame; i < endFrame; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                samples[i * channels + ch] = 0.0f;
            }
        }
    }
}

void WaveformEditor::generateNoise(float startTime, float duration, float amplitude) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t frameCount = static_cast<size_t>(duration * buffer_->getSpec().sampleRate);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(buffer_->getData());
        int channels = buffer_->getSpec().getChannelCount();

        for (size_t i = startFrame; i < startFrame + frameCount && i < buffer_->getFrameCount(); ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                samples[i * channels + ch] = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * amplitude;
            }
        }
    }
}

void WaveformEditor::generateTone(float startTime, float duration, float frequency, float amplitude) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t frameCount = static_cast<size_t>(duration * buffer_->getSpec().sampleRate);
    float sampleRate = static_cast<float>(buffer_->getSpec().sampleRate);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* samples = reinterpret_cast<float*>(buffer_->getData());
        int channels = buffer_->getSpec().getChannelCount();

        float phase = 0.0f;
        float phaseIncrement = 2.0f * 3.14159265359f * frequency / sampleRate;

        for (size_t i = startFrame; i < startFrame + frameCount && i < buffer_->getFrameCount(); ++i) {
            float sample = std::sin(phase) * amplitude;

            for (int ch = 0; ch < channels; ++ch) {
                samples[i * channels + ch] = sample;
            }

            phase += phaseIncrement;
            if (phase > 2.0f * 3.14159265359f) {
                phase -= 2.0f * 3.14159265359f;
            }
        }
    }
}

void WaveformEditor::applyEffect(std::shared_ptr<AudioEffect> effect, float startTime, float endTime) {
    if (!buffer_ || !effect) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * buffer_->getSpec().sampleRate);
    size_t frameCount = endFrame - startFrame;

    AudioSpec spec = buffer_->getSpec();
    AudioBuffer tempBuffer;
    tempBuffer.allocate(spec, frameCount);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        const float* srcSamples = reinterpret_cast<const float*>(buffer_->getData());
        float* dstSamples = reinterpret_cast<float*>(tempBuffer.getData());
        int channels = spec.getChannelCount();

        for (size_t i = 0; i < frameCount; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                dstSamples[i * channels + ch] = srcSamples[(startFrame + i) * channels + ch];
            }
        }
    }

    effect->process(tempBuffer);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        float* dstSamples = reinterpret_cast<float*>(buffer_->getData());
        const float* srcSamples = reinterpret_cast<const float*>(tempBuffer.getData());
        int channels = spec.getChannelCount();

        for (size_t i = 0; i < frameCount; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                dstSamples[(startFrame + i) * channels + ch] = srcSamples[i * channels + ch];
            }
        }
    }
}

void WaveformEditor::trimSilence(float threshold) {
    if (!buffer_) {
        return;
    }

    float thresholdLinear = std::pow(10.0f, threshold / 20.0f);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        const float* samples = reinterpret_cast<const float*>(buffer_->getData());
        size_t sampleCount = buffer_->getSampleCount();

        size_t startSample = 0;
        while (startSample < sampleCount && std::abs(samples[startSample]) < thresholdLinear) {
            startSample++;
        }

        size_t endSample = sampleCount - 1;
        while (endSample > startSample && std::abs(samples[endSample]) < thresholdLinear) {
            endSample--;
        }

        if (startSample >= endSample) {
            return;
        }

        size_t newFrameCount = (endSample - startSample) / buffer_->getSpec().getChannelCount();

        AudioSpec spec = buffer_->getSpec();
        AudioBuffer newBuffer;
        newBuffer.allocate(spec, newFrameCount);

        float* dstSamples = reinterpret_cast<float*>(newBuffer.getData());
        for (size_t i = 0; i < (endSample - startSample); ++i) {
            dstSamples[i] = samples[startSample + i];
        }

        *buffer_ = newBuffer;
    }
}

void WaveformEditor::sampleRateConversion(int newSampleRate) {
    if (!buffer_) {
        return;
    }

    buffer_->resample(newSampleRate);
}

void WaveformEditor::mix(const AudioBuffer& other, float startTime, float gain) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);

    if (buffer_->getSpec().format == AudioFormat::Float32 && other.getSpec().format == AudioFormat::Float32) {
        float* dstSamples = reinterpret_cast<float*>(buffer_->getData());
        const float* srcSamples = reinterpret_cast<const float*>(other.getData());

        int dstChannels = buffer_->getSpec().getChannelCount();
        int srcChannels = other.getSpec().getChannelCount();

        size_t framesToMix = std::min(other.getFrameCount(), buffer_->getFrameCount() - startFrame);

        for (size_t i = 0; i < framesToMix; ++i) {
            for (int ch = 0; ch < dstChannels; ++ch) {
                float srcSample = (ch < srcChannels) ? srcSamples[i * srcChannels + ch] : srcSamples[i * srcChannels];
                dstSamples[(startFrame + i) * dstChannels + ch] += srcSample * gain;
            }
        }
    }
}

void WaveformEditor::crop(float startTime, float endTime) {
    if (!buffer_) {
        return;
    }

    size_t startFrame = static_cast<size_t>(startTime * buffer_->getSpec().sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * buffer_->getSpec().sampleRate);
    size_t frameCount = endFrame - startFrame;

    AudioSpec spec = buffer_->getSpec();
    AudioBuffer newBuffer;
    newBuffer.allocate(spec, frameCount);

    if (buffer_->getSpec().format == AudioFormat::Float32) {
        const float* srcSamples = reinterpret_cast<const float*>(buffer_->getData());
        float* dstSamples = reinterpret_cast<float*>(newBuffer.getData());

        size_t sampleCount = frameCount * spec.getChannelCount();
        size_t startSample = startFrame * spec.getChannelCount();

        for (size_t i = 0; i < sampleCount; ++i) {
            dstSamples[i] = srcSamples[startSample + i];
        }
    }

    *buffer_ = newBuffer;
}

void WaveformEditor::duplicate(float startTime, float endTime) {
}

void WaveformEditor::loop(float startTime, float endTime, int count) {
}

void WaveformEditor::crossfade(float startTime, float duration, const AudioBuffer& other) {
}

void WaveformEditor::setSelection(float startTime, float endTime) {
    selectionStart_ = std::min(startTime, endTime);
    selectionEnd_ = std::max(startTime, endTime);
    hasSelection_ = true;
}

void WaveformEditor::getSelection(float& startTime, float& endTime) const {
    startTime = selectionStart_;
    endTime = selectionEnd_;
}

void WaveformEditor::clearSelection() {
    hasSelection_ = false;
    selectionStart_ = 0.0f;
    selectionEnd_ = 0.0f;
}

bool WaveformEditor::hasSelection() const {
    return hasSelection_;
}

float WaveformEditor::getDuration() const {
    if (!buffer_) {
        return 0.0f;
    }
    return static_cast<float>(buffer_->getFrameCount()) / buffer_->getSpec().sampleRate;
}

int WaveformEditor::getSampleRate() const {
    if (!buffer_) {
        return 44100;
    }
    return buffer_->getSpec().sampleRate;
}

}
}