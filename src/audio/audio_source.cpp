#include "audio_source.h"
#include <algorithm>

namespace v3d {
namespace audio {

AudioSource::AudioSource()
    : state_(PlaybackState::Stopped)
    , looping_(false)
    , volume_(1.0f)
    , pitch_(1.0f)
    , pan_(0.0f)
    , position_(0.0f)
    , velocity_(0.0f)
    , direction_(0.0f, 0.0f, -1.0f)
    , coneInnerAngle_(6.28318f)
    , coneOuterAngle_(6.28318f)
    , coneOuterGain_(0.0f)
    , rolloffFactor_(1.0f)
    , referenceDistance_(1.0f)
    , maxDistance_(1000.0f)
    , minGain_(0.0f)
    , maxGain_(1.0f)
    , priority_(0)
    , currentTime_(0.0f) {
}

AudioSource::~AudioSource() {
    stop();
}

void AudioSource::setBuffer(std::shared_ptr<AudioBuffer> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_ = buffer;
    stream_.reset();
}

std::shared_ptr<AudioBuffer> AudioSource::getBuffer() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_;
}

void AudioSource::setStream(std::shared_ptr<AudioStream> stream) {
    std::lock_guard<std::mutex> lock(mutex_);
    stream_ = stream;
    buffer_.reset();
}

std::shared_ptr<AudioStream> AudioSource::getStream() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stream_;
}

void AudioSource::play() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ == PlaybackState::Playing) {
        return;
    }

    if (!buffer_ && !stream_) {
        return;
    }

    state_ = PlaybackState::Playing;
}

void AudioSource::pause() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = PlaybackState::Paused;
}

void AudioSource::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = PlaybackState::Stopped;
    currentTime_ = 0.0f;

    if (stream_) {
        stream_->seek(0);
    }
}

bool AudioSource::isPlaying() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == PlaybackState::Playing;
}

bool AudioSource::isPaused() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == PlaybackState::Paused;
}

bool AudioSource::isStopped() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == PlaybackState::Stopped;
}

void AudioSource::setLooping(bool looping) {
    std::lock_guard<std::mutex> lock(mutex_);
    looping_ = looping;

    if (auto fileStream = std::dynamic_pointer_cast<AudioFileStream>(stream_)) {
        fileStream->setLooping(looping);
    }
}

bool AudioSource::isLooping() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return looping_;
}

void AudioSource::setVolume(float volume) {
    std::lock_guard<std::mutex> lock(mutex_);
    volume_ = std::clamp(volume, 0.0f, 1.0f);
}

float AudioSource::getVolume() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return volume_;
}

void AudioSource::setPitch(float pitch) {
    std::lock_guard<std::mutex> lock(mutex_);
    pitch_ = std::max(0.1f, pitch);
}

float AudioSource::getPitch() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pitch_;
}

void AudioSource::setPan(float pan) {
    std::lock_guard<std::mutex> lock(mutex_);
    pan_ = std::clamp(pan, -1.0f, 1.0f);
}

float AudioSource::getPan() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pan_;
}

void AudioSource::setPosition(const glm::vec3& position) {
    std::lock_guard<std::mutex> lock(mutex_);
    position_ = position;
}

glm::vec3 AudioSource::getPosition() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return position_;
}

void AudioSource::setVelocity(const glm::vec3& velocity) {
    std::lock_guard<std::mutex> lock(mutex_);
    velocity_ = velocity;
}

glm::vec3 AudioSource::getVelocity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return velocity_;
}

void AudioSource::setDirection(const glm::vec3& direction) {
    std::lock_guard<std::mutex> lock(mutex_);
    direction_ = glm::normalize(direction);
}

glm::vec3 AudioSource::getDirection() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return direction_;
}

void AudioSource::setConeInnerAngle(float angle) {
    std::lock_guard<std::mutex> lock(mutex_);
    coneInnerAngle_ = std::max(0.0f, angle);
}

float AudioSource::getConeInnerAngle() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return coneInnerAngle_;
}

void AudioSource::setConeOuterAngle(float angle) {
    std::lock_guard<std::mutex> lock(mutex_);
    coneOuterAngle_ = std::max(0.0f, angle);
}

float AudioSource::getConeOuterAngle() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return coneOuterAngle_;
}

void AudioSource::setConeOuterGain(float gain) {
    std::lock_guard<std::mutex> lock(mutex_);
    coneOuterGain_ = std::clamp(gain, 0.0f, 1.0f);
}

float AudioSource::getConeOuterGain() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return coneOuterGain_;
}

void AudioSource::setRolloffFactor(float factor) {
    std::lock_guard<std::mutex> lock(mutex_);
    rolloffFactor_ = std::max(0.0f, factor);
}

float AudioSource::getRolloffFactor() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return rolloffFactor_;
}

void AudioSource::setReferenceDistance(float distance) {
    std::lock_guard<std::mutex> lock(mutex_);
    referenceDistance_ = std::max(0.001f, distance);
}

float AudioSource::getReferenceDistance() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return referenceDistance_;
}

void AudioSource::setMaxDistance(float distance) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxDistance_ = std::max(0.001f, distance);
}

float AudioSource::getMaxDistance() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return maxDistance_;
}

void AudioSource::setMinGain(float gain) {
    std::lock_guard<std::mutex> lock(mutex_);
    minGain_ = std::clamp(gain, 0.0f, 1.0f);
}

float AudioSource::getMinGain() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return minGain_;
}

void AudioSource::setMaxGain(float gain) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxGain_ = std::clamp(gain, 0.0f, 1.0f);
}

float AudioSource::getMaxGain() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return maxGain_;
}

void AudioSource::setPriority(int priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    priority_ = priority;
}

int AudioSource::getPriority() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return priority_;
}

float AudioSource::getProgress() const {
    std::lock_guard<std::mutex> lock(mutex_);

    float duration = getDuration();
    if (duration > 0.0f) {
        return currentTime_ / duration;
    }
    return 0.0f;
}

float AudioSource::getDuration() const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (buffer_) {
        return static_cast<float>(buffer_->getFrameCount()) / buffer_->getSpec().sampleRate;
    } else if (stream_) {
        return static_cast<float>(stream_->getTotalFrames()) / stream_->getSpec().sampleRate;
    }

    return 0.0f;
}

void AudioSource::seek(float time) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (stream_) {
        size_t frame = static_cast<size_t>(time * stream_->getSpec().sampleRate);
        stream_->seek(frame);
    }

    currentTime_ = time;
}

void AudioSource::update(float deltaTime) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != PlaybackState::Playing) {
        return;
    }

    currentTime_ += deltaTime * pitch_;

    if (buffer_) {
        float duration = getDuration();
        if (currentTime_ >= duration) {
            if (looping_) {
                currentTime_ = fmod(currentTime_, duration);
            } else {
                state_ = PlaybackState::Stopped;
                currentTime_ = duration;
            }
        }
    } else if (stream_) {
        if (stream_->tell() >= stream_->getTotalFrames() && !looping_) {
            state_ = PlaybackState::Stopped;
        }
    }
}

AudioListener::AudioListener()
    : position_(0.0f)
    , velocity_(0.0f)
    , forward_(0.0f, 0.0f, -1.0f)
    , up_(0.0f, 1.0f, 0.0f)
    , gain_(1.0f) {
}

AudioListener::~AudioListener() {
}

void AudioListener::setPosition(const glm::vec3& position) {
    position_ = position;
}

glm::vec3 AudioListener::getPosition() const {
    return position_;
}

void AudioListener::setVelocity(const glm::vec3& velocity) {
    velocity_ = velocity;
}

glm::vec3 AudioListener::getVelocity() const {
    return velocity_;
}

void AudioListener::setOrientation(const glm::vec3& forward, const glm::vec3& up) {
    forward_ = glm::normalize(forward);
    up_ = glm::normalize(up);
}

void AudioListener::getOrientation(glm::vec3& forward, glm::vec3& up) const {
    forward = forward_;
    up = up_;
}

void AudioListener::setGain(float gain) {
    gain_ = std::clamp(gain, 0.0f, 1.0f);
}

float AudioListener::getGain() const {
    return gain_;
}

void AudioListener::update(float deltaTime) {
}

}
}