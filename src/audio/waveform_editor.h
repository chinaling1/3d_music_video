#pragma once

#include "audio_buffer.h"
#include <vector>
#include <memory>

namespace v3d {
namespace audio {

class AudioEffect;

class WaveformEditor {
public:
    WaveformEditor();
    ~WaveformEditor();

    void setBuffer(std::shared_ptr<AudioBuffer> buffer);
    std::shared_ptr<AudioBuffer> getBuffer() const;

    std::vector<float> getWaveformData(int channel = 0, int resolution = 1000) const;

    void cut(float startTime, float endTime);
    void copy(float startTime, float endTime);
    void paste(float startTime, const AudioBuffer& clipboard);

    void deleteRange(float startTime, float endTime);

    void insertSilence(float startTime, float duration);

    void normalize(float targetLevel = 1.0f);
    void amplify(float gain);

    void fade(float startTime, float endTime, float startLevel, float endLevel);
    void fadeIn(float startTime, float duration);
    void fadeOut(float startTime, float duration);

    void reverse(float startTime, float endTime);
    void invert(float startTime, float endTime);

    void timeStretch(float startTime, float endTime, float ratio);
    void pitchShift(float startTime, float endTime, float semitones);

    void silence(float startTime, float endTime);

    void generateNoise(float startTime, float duration, float amplitude = 1.0f);
    void generateTone(float startTime, float duration, float frequency, float amplitude = 1.0f);

    void applyEffect(std::shared_ptr<AudioEffect> effect, float startTime, float endTime);

    void trimSilence(float threshold = -60.0f);

    void sampleRateConversion(int newSampleRate);

    void mix(const AudioBuffer& other, float startTime, float gain = 1.0f);

    void crop(float startTime, float endTime);

    void duplicate(float startTime, float endTime);

    void loop(float startTime, float endTime, int count);

    void crossfade(float startTime, float duration, const AudioBuffer& other);

    void setSelection(float startTime, float endTime);
    void getSelection(float& startTime, float& endTime) const;

    void clearSelection();

    bool hasSelection() const;

    float getDuration() const;
    int getSampleRate() const;

private:
    std::shared_ptr<AudioBuffer> buffer_;
    float selectionStart_;
    float selectionEnd_;
    bool hasSelection_;
};

}
}