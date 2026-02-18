#include "audio_mixer.h"
#include <cstring>
#include <algorithm>

namespace v3d {
namespace audio {

AudioMixer::AudioMixer() {
}

AudioMixer::~AudioMixer() {
}

void AudioMixer::addBus(const std::string& name) {
    Bus bus;
    bus.name = name;
    bus.volume = 1.0f;
    bus.muted = false;
    bus.solo = false;
    buses_[name] = bus;
}

void AudioMixer::removeBus(const std::string& name) {
    buses_.erase(name);
}

void AudioMixer::setBusVolume(const std::string& name, float volume) {
    auto it = buses_.find(name);
    if (it != buses_.end()) {
        it->second.volume = std::clamp(volume, 0.0f, 1.0f);
    }
}

float AudioMixer::getBusVolume(const std::string& name) const {
    auto it = buses_.find(name);
    return it != buses_.end() ? it->second.volume : 1.0f;
}

void AudioMixer::setBusMute(const std::string& name, bool mute) {
    auto it = buses_.find(name);
    if (it != buses_.end()) {
        it->second.muted = mute;
    }
}

bool AudioMixer::isBusMuted(const std::string& name) const {
    auto it = buses_.find(name);
    return it != buses_.end() ? it->second.muted : false;
}

void AudioMixer::setBusSolo(const std::string& name, bool solo) {
    auto it = buses_.find(name);
    if (it != buses_.end()) {
        it->second.solo = solo;
    }
}

bool AudioMixer::isBusSolo(const std::string& name) const {
    auto it = buses_.find(name);
    return it != buses_.end() ? it->second.solo : false;
}

void AudioMixer::addSourceToBus(const std::string& busName, std::shared_ptr<AudioSource> source) {
    auto it = buses_.find(busName);
    if (it != buses_.end()) {
        it->second.sources.push_back(source);
    }
}

void AudioMixer::removeSourceFromBus(const std::string& busName, std::shared_ptr<AudioSource> source) {
    auto it = buses_.find(busName);
    if (it != buses_.end()) {
        auto& sources = it->second.sources;
        sources.erase(std::remove_if(sources.begin(), sources.end(),
            [&source](const std::weak_ptr<AudioSource>& weak) {
                return weak.lock() == source;
            }), sources.end());
    }
}

void AudioMixer::process(void* output, size_t frames) {
    memset(output, 0, frames * 4);

    bool hasSolo = false;
    for (const auto& pair : buses_) {
        if (pair.second.solo) {
            hasSolo = true;
            break;
        }
    }

    for (auto& pair : buses_) {
        Bus& bus = pair.second;

        if (bus.muted || (hasSolo && !bus.solo)) {
            continue;
        }

        for (auto& weakSource : bus.sources) {
            auto source = weakSource.lock();
            if (source && source->isPlaying()) {
            }
        }
    }
}

}
}