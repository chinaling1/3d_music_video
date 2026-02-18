#pragma once

#include "audio_source.h"
#include "audio_buffer.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace v3d {
namespace audio {

class AudioEngine {
public:
    static AudioEngine& getInstance();

    bool initialize(int sampleRate = 44100, int bufferSize = 512, AudioChannelLayout channels = AudioChannelLayout::Stereo);
    void shutdown();

    bool isInitialized() const;

    void update();

    std::shared_ptr<AudioSource> createSource();
    void destroySource(AudioSource* source);

    std::shared_ptr<AudioListener> createListener();
    void destroyListener(AudioListener* listener);

    void setMasterVolume(float volume);
    float getMasterVolume() const;

    void setMasterGain(float gain);
    float getMasterGain() const;

    const AudioSpec& getOutputSpec() const;

    void setCallback(std::function<void(void*, size_t)> callback);

    void suspend();
    void resume();

    int getSampleRate() const;
    int getBufferSize() const;

    void setDopplerFactor(float factor);
    float getDopplerFactor() const;

    void setSpeedOfSound(float speed);
    float getSpeedOfSound() const;

private:
    AudioEngine();
    ~AudioEngine();

    void audioThread();
    void processAudio(void* output, size_t frames);

    AudioSpec outputSpec_;
    std::atomic<bool> initialized_;
    std::atomic<bool> running_;
    std::atomic<bool> suspended_;

    std::vector<std::shared_ptr<AudioSource>> sources_;
    std::vector<std::shared_ptr<AudioListener>> listeners_;

    float masterVolume_;
    float masterGain_;

    float dopplerFactor_;
    float speedOfSound_;

    std::function<void(void*, size_t)> audioCallback_;

    std::thread audioThread_;
    std::mutex mutex_;
    std::condition_variable condition_;

    std::vector<uint8_t> mixBuffer_;
};

}
}