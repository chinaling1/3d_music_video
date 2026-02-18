#pragma once

#include "audio_buffer.h"
#include <glm/glm.hpp>
#include <memory>
#include <atomic>
#include <mutex>

namespace v3d {
namespace audio {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

class AudioSource {
public:
    AudioSource();
    ~AudioSource();

    void setBuffer(std::shared_ptr<AudioBuffer> buffer);
    std::shared_ptr<AudioBuffer> getBuffer() const;

    void setStream(std::shared_ptr<AudioStream> stream);
    std::shared_ptr<AudioStream> getStream() const;

    void play();
    void pause();
    void stop();

    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    void setLooping(bool looping);
    bool isLooping() const;

    void setVolume(float volume);
    float getVolume() const;

    void setPitch(float pitch);
    float getPitch() const;

    void setPan(float pan);
    float getPan() const;

    void setPosition(const glm::vec3& position);
    glm::vec3 getPosition() const;

    void setVelocity(const glm::vec3& velocity);
    glm::vec3 getVelocity() const;

    void setDirection(const glm::vec3& direction);
    glm::vec3 getDirection() const;

    void setConeInnerAngle(float angle);
    float getConeInnerAngle() const;

    void setConeOuterAngle(float angle);
    float getConeOuterAngle() const;

    void setConeOuterGain(float gain);
    float getConeOuterGain() const;

    void setRolloffFactor(float factor);
    float getRolloffFactor() const;

    void setReferenceDistance(float distance);
    float getReferenceDistance() const;

    void setMaxDistance(float distance);
    float getMaxDistance() const;

    void setMinGain(float gain);
    float getMinGain() const;

    void setMaxGain(float gain);
    float getMaxGain() const;

    void setPriority(int priority);
    int getPriority() const;

    float getProgress() const;
    float getDuration() const;

    void seek(float time);

    void update(float deltaTime);

private:
    std::shared_ptr<AudioBuffer> buffer_;
    std::shared_ptr<AudioStream> stream_;

    PlaybackState state_;
    bool looping_;
    float volume_;
    float pitch_;
    float pan_;

    glm::vec3 position_;
    glm::vec3 velocity_;
    glm::vec3 direction_;

    float coneInnerAngle_;
    float coneOuterAngle_;
    float coneOuterGain_;

    float rolloffFactor_;
    float referenceDistance_;
    float maxDistance_;

    float minGain_;
    float maxGain_;

    int priority_;

    float currentTime_;
    mutable std::mutex mutex_;
};

class AudioListener {
public:
    AudioListener();
    ~AudioListener();

    void setPosition(const glm::vec3& position);
    glm::vec3 getPosition() const;

    void setVelocity(const glm::vec3& velocity);
    glm::vec3 getVelocity() const;

    void setOrientation(const glm::vec3& forward, const glm::vec3& up);
    void getOrientation(glm::vec3& forward, glm::vec3& up) const;

    void setGain(float gain);
    float getGain() const;

    void update(float deltaTime);

private:
    glm::vec3 position_;
    glm::vec3 velocity_;
    glm::vec3 forward_;
    glm::vec3 up_;
    float gain_;
};

}
}