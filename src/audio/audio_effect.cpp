#include "audio_effect.h"
#include <cmath>
#include <algorithm>

namespace v3d {
namespace audio {

AudioEffect::AudioEffect()
    : enabled_(true)
    , bypass_(false)
    , mix_(1.0f) {
}

AudioEffect::~AudioEffect() {
}

void AudioEffect::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool AudioEffect::isEnabled() const {
    return enabled_;
}

void AudioEffect::setBypass(bool bypass) {
    bypass_ = bypass;
}

bool AudioEffect::isBypassed() const {
    return bypass_;
}

void AudioEffect::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float AudioEffect::getMix() const {
    return mix_;
}

Equalizer::Equalizer() {
    bands_.resize(10);

    float frequencies[] = {32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
    for (size_t i = 0; i < bands_.size(); ++i) {
        bands_[i].frequency = frequencies[i];
        bands_[i].gain = 0.0f;
        bands_[i].Q = 1.414f;
        bands_[i].enabled = true;
    }
}

Equalizer::~Equalizer() {
}

void Equalizer::process(AudioBuffer& buffer) {
    if (bypass_ || !enabled_) {
        return;
    }

    if (buffer.getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer.getData());
    size_t sampleCount = buffer.getSampleCount();
    int channels = buffer.getSpec().getChannelCount();

    for (size_t i = 0; i < sampleCount; ++i) {
        float sample = samples[i];

        for (const auto& band : bands_) {
            if (band.enabled) {
                float gain = std::pow(10.0f, band.gain / 20.0f);
                sample *= gain;
            }
        }

        samples[i] = sample;
    }
}

void Equalizer::setBandCount(int count) {
    bands_.resize(std::max(1, count));
}

int Equalizer::getBandCount() const {
    return static_cast<int>(bands_.size());
}

void Equalizer::setBand(int index, const Band& band) {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        bands_[index] = band;
    }
}

Equalizer::Band Equalizer::getBand(int index) const {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        return bands_[index];
    }
    return Band();
}

void Equalizer::setBandGain(int index, float gain) {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        bands_[index].gain = std::clamp(gain, -12.0f, 12.0f);
    }
}

float Equalizer::getBandGain(int index) const {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        return bands_[index].gain;
    }
    return 0.0f;
}

void Equalizer::setBandFrequency(int index, float frequency) {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        bands_[index].frequency = std::max(20.0f, frequency);
    }
}

float Equalizer::getBandFrequency(int index) const {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        return bands_[index].frequency;
    }
    return 1000.0f;
}

void Equalizer::setBandQ(int index, float Q) {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        bands_[index].Q = std::max(0.1f, Q);
    }
}

float Equalizer::getBandQ(int index) const {
    if (index >= 0 && index < static_cast<int>(bands_.size())) {
        return bands_[index].Q;
    }
    return 1.414f;
}

Compressor::Compressor()
    : threshold_(-20.0f)
    , ratio_(4.0f)
    , attack_(0.005f)
    , release_(0.1f)
    , knee_(6.0f)
    , makeupGain_(0.0f)
    , envelope_(0.0f) {
}

Compressor::~Compressor() {
}

void Compressor::process(AudioBuffer& buffer) {
    if (bypass_ || !enabled_) {
        return;
    }

    if (buffer.getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer.getData());
    size_t sampleCount = buffer.getSampleCount();
    int channels = buffer.getSpec().getChannelCount();
    float sampleRate = static_cast<float>(buffer.getSpec().sampleRate);

    float attackCoeff = std::exp(-1.0f / (attack_ * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (release_ * sampleRate));

    float thresholdLinear = std::pow(10.0f, threshold_ / 20.0f);

    for (size_t i = 0; i < sampleCount; ++i) {
        float sample = samples[i];
        float inputLevel = std::abs(sample);

        float targetEnvelope = inputLevel;

        if (targetEnvelope > envelope_) {
            envelope_ = attackCoeff * envelope_ + (1.0f - attackCoeff) * targetEnvelope;
        } else {
            envelope_ = releaseCoeff * envelope_ + (1.0f - releaseCoeff) * targetEnvelope;
        }

        float gain = 1.0f;

        if (envelope_ > thresholdLinear) {
            float excessDB = 20.0f * std::log10(envelope_ / thresholdLinear);
            float gainReduction = excessDB * (1.0f - 1.0f / ratio_);
            gain = std::pow(10.0f, -gainReduction / 20.0f);
        }

        float makeupLinear = std::pow(10.0f, makeupGain_ / 20.0f);
        samples[i] = sample * gain * makeupLinear;
    }
}

void Compressor::setThreshold(float threshold) {
    threshold_ = std::clamp(threshold, -60.0f, 0.0f);
}

float Compressor::getThreshold() const {
    return threshold_;
}

void Compressor::setRatio(float ratio) {
    ratio_ = std::max(1.0f, ratio);
}

float Compressor::getRatio() const {
    return ratio_;
}

void Compressor::setAttack(float attack) {
    attack_ = std::max(0.0f, attack);
}

float Compressor::getAttack() const {
    return attack_;
}

void Compressor::setRelease(float release) {
    release_ = std::max(0.0f, release);
}

float Compressor::getRelease() const {
    return release_;
}

void Compressor::setKnee(float knee) {
    knee_ = std::max(0.0f, knee);
}

float Compressor::getKnee() const {
    return knee_;
}

void Compressor::setMakeupGain(float gain) {
    makeupGain_ = std::clamp(gain, 0.0f, 24.0f);
}

float Compressor::getMakeupGain() const {
    return makeupGain_;
}

Reverb::Reverb()
    : roomSize_(0.5f)
    , damping_(0.5f)
    , wetLevel_(0.3f)
    , dryLevel_(0.7f)
    , width_(1.0f) {
    delayLines_.resize(8);
    delayTimes_.resize(8);
    feedbackGains_.resize(8);

    for (size_t i = 0; i < delayLines_.size(); ++i) {
        delayLines_[i].resize(44100);
        delayTimes_[i] = 0.05f + static_cast<float>(i) * 0.01f;
        feedbackGains_[i] = 0.5f;
    }
}

Reverb::~Reverb() {
}

void Reverb::process(AudioBuffer& buffer) {
    if (bypass_ || !enabled_) {
        return;
    }

    if (buffer.getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer.getData());
    size_t frameCount = buffer.getFrameCount();
    int channels = buffer.getSpec().getChannelCount();
    float sampleRate = static_cast<float>(buffer.getSpec().sampleRate);

    std::vector<float> wetSamples(frameCount * channels, 0.0f);

    for (size_t frame = 0; frame < frameCount; ++frame) {
        for (int ch = 0; ch < channels; ++ch) {
            float inputSample = samples[frame * channels + ch];
            float wetSample = 0.0f;

            for (size_t i = 0; i < delayLines_.size(); ++i) {
                size_t delaySamples = static_cast<size_t>(delayTimes_[i] * sampleRate);
                size_t readIndex = (frame + delayLines_.size() * 44100 - delaySamples) % delayLines_[i].size();

                float delayedSample = delayLines_[i][readIndex];
                wetSample += delayedSample * feedbackGains_[i];
            }

            wetSample *= wetLevel_;
            samples[frame * channels + ch] = inputSample * dryLevel_ + wetSample;

            for (size_t i = 0; i < delayLines_.size(); ++i) {
                size_t writeIndex = (frame + delayLines_.size() * 44100) % delayLines_[i].size();
                delayLines_[i][writeIndex] = inputSample + wetSample * damping_;
            }
        }
    }
}

void Reverb::setRoomSize(float size) {
    roomSize_ = std::clamp(size, 0.0f, 1.0f);
}

float Reverb::getRoomSize() const {
    return roomSize_;
}

void Reverb::setDamping(float damping) {
    damping_ = std::clamp(damping, 0.0f, 1.0f);
}

float Reverb::getDamping() const {
    return damping_;
}

void Reverb::setWetLevel(float level) {
    wetLevel_ = std::clamp(level, 0.0f, 1.0f);
}

float Reverb::getWetLevel() const {
    return wetLevel_;
}

void Reverb::setDryLevel(float level) {
    dryLevel_ = std::clamp(level, 0.0f, 1.0f);
}

float Reverb::getDryLevel() const {
    return dryLevel_;
}

void Reverb::setWidth(float width) {
    width_ = std::clamp(width, 0.0f, 1.0f);
}

float Reverb::getWidth() const {
    return width_;
}

Delay::Delay()
    : delayTime_(0.3f)
    , feedback_(0.5f)
    , writeIndex_(0) {
    delayBuffer_.resize(44100 * 2);
}

Delay::~Delay() {
}

void Delay::process(AudioBuffer& buffer) {
    if (bypass_ || !enabled_) {
        return;
    }

    if (buffer.getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer.getData());
    size_t frameCount = buffer.getFrameCount();
    int channels = buffer.getSpec().getChannelCount();
    float sampleRate = static_cast<float>(buffer.getSpec().sampleRate);

    size_t delaySamples = static_cast<size_t>(delayTime_ * sampleRate);

    for (size_t frame = 0; frame < frameCount; ++frame) {
        for (int ch = 0; ch < channels; ++ch) {
            float inputSample = samples[frame * channels + ch];

            size_t readIndex = (writeIndex_ - delaySamples + delayBuffer_.size()) % delayBuffer_.size();
            float delayedSample = delayBuffer_[readIndex];

            float outputSample = inputSample + delayedSample * mix_;
            samples[frame * channels + ch] = outputSample;

            delayBuffer_[writeIndex_] = inputSample + delayedSample * feedback_;
        }

        writeIndex_ = (writeIndex_ + 1) % delayBuffer_.size();
    }
}

void Delay::setDelayTime(float time) {
    delayTime_ = std::max(0.0f, time);
}

float Delay::getDelayTime() const {
    return delayTime_;
}

void Delay::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.95f);
}

float Delay::getFeedback() const {
    return feedback_;
}

void Delay::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Delay::getMix() const {
    return mix_;
}

Chorus::Chorus()
    : rate_(1.5f)
    , depth_(0.002f)
    , phase_(0.0f)
    , writeIndex_(0) {
    delayBuffer_.resize(44100);
}

Chorus::~Chorus() {
}

void Chorus::process(AudioBuffer& buffer) {
    if (bypass_ || !enabled_) {
        return;
    }

    if (buffer.getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer.getData());
    size_t frameCount = buffer.getFrameCount();
    int channels = buffer.getSpec().getChannelCount();
    float sampleRate = static_cast<float>(buffer.getSpec().sampleRate);

    for (size_t frame = 0; frame < frameCount; ++frame) {
        float lfo = std::sin(phase_);
        float modDelay = depth_ * (1.0f + lfo);
        size_t delaySamples = static_cast<size_t>(modDelay * sampleRate);

        for (int ch = 0; ch < channels; ++ch) {
            float inputSample = samples[frame * channels + ch];

            size_t readIndex = (writeIndex_ - delaySamples + delayBuffer_.size()) % delayBuffer_.size();
            float delayedSample = delayBuffer_[readIndex];

            float outputSample = inputSample * (1.0f - mix_) + delayedSample * mix_;
            samples[frame * channels + ch] = outputSample;

            delayBuffer_[writeIndex_] = inputSample;
        }

        writeIndex_ = (writeIndex_ + 1) % delayBuffer_.size();
        phase_ += 2.0f * 3.14159265359f * rate_ / sampleRate;
    }
}

void Chorus::setRate(float rate) {
    rate_ = std::max(0.1f, rate);
}

float Chorus::getRate() const {
    return rate_;
}

void Chorus::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 0.01f);
}

float Chorus::getDepth() const {
    return depth_;
}

void Chorus::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Chorus::getMix() const {
    return mix_;
}

Distortion::Distortion()
    : type_(Type::HardClip)
    , drive_(1.0f)
    , tone_(1.0f) {
}

Distortion::~Distortion() {
}

void Distortion::process(AudioBuffer& buffer) {
    if (bypass_ || !enabled_) {
        return;
    }

    if (buffer.getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer.getData());
    size_t sampleCount = buffer.getSampleCount();

    for (size_t i = 0; i < sampleCount; ++i) {
        float sample = samples[i] * drive_;

        switch (type_) {
            case Type::HardClip:
                sample = std::clamp(sample, -1.0f, 1.0f);
                break;
            case Type::SoftClip:
                sample = std::tanh(sample);
                break;
            case Type::Overdrive:
                sample = sample * (2.0f - std::abs(sample));
                break;
            case Type::Fuzz:
                sample = std::sin(sample * 3.14159265359f / 2.0f);
                break;
        }

        samples[i] = sample * tone_;
    }
}

void Distortion::setType(Type type) {
    type_ = type;
}

Distortion::Type Distortion::getType() const {
    return type_;
}

void Distortion::setDrive(float drive) {
    drive_ = std::max(1.0f, drive);
}

float Distortion::getDrive() const {
    return drive_;
}

void Distortion::setTone(float tone) {
    tone_ = std::max(0.0f, tone);
}

float Distortion::getTone() const {
    return tone_;
}

Filter::Filter()
    : type_(Type::LowPass)
    , frequency_(1000.0f)
    , Q_(0.707f)
    , gain_(0.0f) {
}

Filter::~Filter() {
}

void Filter::process(AudioBuffer& buffer) {
    if (bypass_ || !enabled_) {
        return;
    }

    if (buffer.getSpec().format != AudioFormat::Float32) {
        return;
    }

    float* samples = reinterpret_cast<float*>(buffer.getData());
    size_t sampleCount = buffer.getSampleCount();
    int channels = buffer.getSpec().getChannelCount();
    float sampleRate = static_cast<float>(buffer.getSpec().sampleRate);

    state1_.resize(channels, 0.0f);
    state2_.resize(channels, 0.0f);

    float omega = 2.0f * 3.14159265359f * frequency_ / sampleRate;
    float sinOmega = std::sin(omega);
    float cosOmega = std::cos(omega);
    float alpha = sinOmega / (2.0f * Q_);

    float b0, b1, b2, a0, a1, a2;

    switch (type_) {
        case Type::LowPass:
            b0 = (1.0f - cosOmega) / 2.0f;
            b1 = 1.0f - cosOmega;
            b2 = (1.0f - cosOmega) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosOmega;
            a2 = 1.0f - alpha;
            break;
        case Type::HighPass:
            b0 = (1.0f + cosOmega) / 2.0f;
            b1 = -(1.0f + cosOmega);
            b2 = (1.0f + cosOmega) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosOmega;
            a2 = 1.0f - alpha;
            break;
        case Type::BandPass:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosOmega;
            a2 = 1.0f - alpha;
            break;
        default:
            b0 = 1.0f;
            b1 = 0.0f;
            b2 = 0.0f;
            a0 = 1.0f;
            a1 = 0.0f;
            a2 = 0.0f;
            break;
    }

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    for (size_t i = 0; i < sampleCount; ++i) {
        for (int ch = 0; ch < channels; ++ch) {
            int idx = i * channels + ch;
            float input = samples[idx];
            float output = b0 * input + b1 * state1_[ch] + b2 * state2_[ch] -
                           a1 * state1_[ch] - a2 * state2_[ch];

            state2_[ch] = state1_[ch];
            state1_[ch] = input;

            samples[idx] = output;
        }
    }
}

void Filter::setType(Type type) {
    type_ = type;
}

Filter::Type Filter::getType() const {
    return type_;
}

void Filter::setFrequency(float frequency) {
    frequency_ = std::max(20.0f, frequency);
}

float Filter::getFrequency() const {
    return frequency_;
}

void Filter::setQ(float Q) {
    Q_ = std::max(0.1f, Q);
}

float Filter::getQ() const {
    return Q_;
}

void Filter::setGain(float gain) {
    gain_ = gain;
}

float Filter::getGain() const {
    return gain_;
}

EffectChain::EffectChain()
    : enabled_(true) {
}

EffectChain::~EffectChain() {
}

void EffectChain::addEffect(std::shared_ptr<AudioEffect> effect) {
    effects_.push_back(effect);
}

void EffectChain::removeEffect(AudioEffect* effect) {
    effects_.erase(std::remove_if(effects_.begin(), effects_.end(),
        [effect](const std::shared_ptr<AudioEffect>& e) {
            return e.get() == effect;
        }), effects_.end());
}

void EffectChain::clearEffects() {
    effects_.clear();
}

void EffectChain::process(AudioBuffer& buffer) {
    if (!enabled_) {
        return;
    }

    for (auto& effect : effects_) {
        if (effect && effect->isEnabled() && !effect->isBypassed()) {
            effect->process(buffer);
        }
    }
}

void EffectChain::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool EffectChain::isEnabled() const {
    return enabled_;
}

}
}