#include "audio_engine.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace v3d {
namespace audio {

AudioEngine& AudioEngine::getInstance() {
    static AudioEngine instance;
    return instance;
}

AudioEngine::AudioEngine()
    : initialized_(false)
    , running_(false)
    , suspended_(false)
    , masterVolume_(1.0f)
    , masterGain_(1.0f)
    , dopplerFactor_(1.0f)
    , speedOfSound_(343.3f) {
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize(int sampleRate, int bufferSize, AudioChannelLayout channels) {
    if (initialized_) {
        return true;
    }

    outputSpec_.sampleRate = sampleRate;
    outputSpec_.format = AudioFormat::Float32;
    outputSpec_.channels = channels;
    outputSpec_.frameSize = outputSpec_.getBytesPerFrame();

    size_t mixBufferSize = bufferSize * outputSpec_.getBytesPerFrame();
    mixBuffer_.resize(mixBufferSize);

    running_ = true;
    audioThread_ = std::thread(&AudioEngine::audioThread, this);

    initialized_ = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!initialized_) {
        return;
    }

    running_ = false;
    condition_.notify_all();

    if (audioThread_.joinable()) {
        audioThread_.join();
    }

    sources_.clear();
    listeners_.clear();
    mixBuffer_.clear();

    initialized_ = false;
}

bool AudioEngine::isInitialized() const {
    return initialized_;
}

void AudioEngine::update() {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& source : sources_) {
        source->update(0.016f);
    }

    for (auto& listener : listeners_) {
        listener->update(0.016f);
    }
}

std::shared_ptr<AudioSource> AudioEngine::createSource() {
    std::lock_guard<std::mutex> lock(mutex_);

    auto source = std::make_shared<AudioSource>();
    sources_.push_back(source);

    return source;
}

void AudioEngine::destroySource(AudioSource* source) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::remove_if(sources_.begin(), sources_.end(),
        [source](const std::shared_ptr<AudioSource>& s) {
            return s.get() == source;
        });

    sources_.erase(it, sources_.end());
}

std::shared_ptr<AudioListener> AudioEngine::createListener() {
    std::lock_guard<std::mutex> lock(mutex_);

    auto listener = std::make_shared<AudioListener>();
    listeners_.push_back(listener);

    return listener;
}

void AudioEngine::destroyListener(AudioListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::remove_if(listeners_.begin(), listeners_.end(),
        [listener](const std::shared_ptr<AudioListener>& l) {
            return l.get() == listener;
        });

    listeners_.erase(it, listeners_.end());
}

void AudioEngine::setMasterVolume(float volume) {
    masterVolume_ = std::clamp(volume, 0.0f, 1.0f);
}

float AudioEngine::getMasterVolume() const {
    return masterVolume_;
}

void AudioEngine::setMasterGain(float gain) {
    masterGain_ = std::clamp(gain, 0.0f, 1.0f);
}

float AudioEngine::getMasterGain() const {
    return masterGain_;
}

const AudioSpec& AudioEngine::getOutputSpec() const {
    return outputSpec_;
}

void AudioEngine::setCallback(std::function<void(void*, size_t)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    audioCallback_ = callback;
}

void AudioEngine::suspend() {
    suspended_ = true;
}

void AudioEngine::resume() {
    suspended_ = false;
    condition_.notify_one();
}

int AudioEngine::getSampleRate() const {
    return outputSpec_.sampleRate;
}

int AudioEngine::getBufferSize() const {
    return mixBuffer_.size() / outputSpec_.getBytesPerFrame();
}

void AudioEngine::setDopplerFactor(float factor) {
    dopplerFactor_ = std::max(0.0f, factor);
}

float AudioEngine::getDopplerFactor() const {
    return dopplerFactor_;
}

void AudioEngine::setSpeedOfSound(float speed) {
    speedOfSound_ = std::max(0.1f, speed);
}

float AudioEngine::getSpeedOfSound() const {
    return speedOfSound_;
}

void AudioEngine::audioThread() {
    while (running_) {
        if (suspended_) {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return !suspended_ || !running_; });
            continue;
        }

        processAudio(mixBuffer_.data(), mixBuffer_.size() / outputSpec_.getBytesPerFrame());

        if (audioCallback_) {
            audioCallback_(mixBuffer_.data(), mixBuffer_.size());
        }
    }
}

void AudioEngine::processAudio(void* output, size_t frames) {
    std::lock_guard<std::mutex> lock(mutex_);

    memset(output, 0, frames * outputSpec_.getBytesPerFrame());

    int channelCount = outputSpec_.getChannelCount();

    if (listeners_.empty()) {
        return;
    }

    const AudioListener& listener = *listeners_[0];
    glm::vec3 listenerPos = listener.getPosition();
    glm::vec3 listenerVel = listener.getVelocity();
    glm::vec3 listenerForward, listenerUp;
    listener.getOrientation(listenerForward, listenerUp);

    for (const auto& source : sources_) {
        if (!source->isPlaying()) {
            continue;
        }

        float sourceVolume = source->getVolume();
        float sourcePitch = source->getPitch();
        float sourcePan = source->getPan();

        glm::vec3 sourcePos = source->getPosition();
        glm::vec3 sourceVel = source->getVelocity();

        float distance = glm::length(sourcePos - listenerPos);

        float attenuation = 1.0f;
        if (distance > source->getReferenceDistance()) {
            attenuation = source->getReferenceDistance() /
                         (source->getReferenceDistance() +
                          source->getRolloffFactor() * (distance - source->getReferenceDistance()));
        }

        attenuation = std::clamp(attenuation, source->getMinGain(), source->getMaxGain());

        float finalVolume = sourceVolume * attenuation * masterVolume_ * masterGain_;

        if (finalVolume < 0.0001f) {
            continue;
        }

        if (outputSpec_.format == AudioFormat::Float32) {
            float* outputSamples = reinterpret_cast<float*>(output);

            if (source->getBuffer()) {
                const AudioBuffer& buffer = *source->getBuffer();
                const float* bufferSamples = reinterpret_cast<const float*>(buffer.getData());

                size_t bufferChannels = buffer.getSpec().getChannelCount();
                size_t bufferFrames = buffer.getFrameCount();

                float currentTime = source->getProgress();
                size_t startFrame = static_cast<size_t>(currentTime * bufferFrames);

                for (size_t frame = 0; frame < frames; ++frame) {
                    size_t bufferFrame = (startFrame + frame) % bufferFrames;

                    for (int ch = 0; ch < channelCount; ++ch) {
                        float sample = 0.0f;
                        if (ch < static_cast<int>(bufferChannels)) {
                            sample = bufferSamples[bufferFrame * bufferChannels + ch];
                        } else {
                            sample = bufferSamples[bufferFrame * bufferChannels];
                        }

                        float panGain = 1.0f;
                        if (channelCount == 2) {
                            if (ch == 0) {
                                panGain = 1.0f - sourcePan * 0.5f;
                            } else {
                                panGain = 1.0f + sourcePan * 0.5f;
                            }
                        }

                        outputSamples[frame * channelCount + ch] += sample * finalVolume * panGain;
                    }
                }
            }
        }
    }
}

}
}