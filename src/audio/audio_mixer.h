#pragma once

#include "audio_engine.h"

namespace v3d {
namespace audio {

class AudioMixer {
public:
    AudioMixer();
    ~AudioMixer();

    void addBus(const std::string& name);
    void removeBus(const std::string& name);

    void setBusVolume(const std::string& name, float volume);
    float getBusVolume(const std::string& name) const;

    void setBusMute(const std::string& name, bool mute);
    bool isBusMuted(const std::string& name) const;

    void setBusSolo(const std::string& name, bool solo);
    bool isBusSolo(const std::string& name) const;

    void addSourceToBus(const std::string& busName, std::shared_ptr<AudioSource> source);
    void removeSourceFromBus(const std::string& busName, std::shared_ptr<AudioSource> source);

    void process(void* output, size_t frames);

private:
    struct Bus {
        std::string name;
        float volume;
        bool muted;
        bool solo;
        std::vector<std::weak_ptr<AudioSource>> sources;
    };

    std::unordered_map<std::string, Bus> buses_;
};

}
}