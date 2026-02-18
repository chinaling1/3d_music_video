#pragma once

#include "audio_buffer.h"
#include <memory>
#include <vector>
#include <functional>

namespace v3d {
namespace audio {

class AudioEffect {
public:
    AudioEffect();
    virtual ~AudioEffect();

    virtual void process(AudioBuffer& buffer) = 0;

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setBypass(bool bypass);
    bool isBypassed() const;

    void setMix(float mix);
    float getMix() const;

protected:
    bool enabled_;
    bool bypass_;
    float mix_;
};

class Equalizer : public AudioEffect {
public:
    struct Band {
        float frequency;
        float gain;
        float Q;
        bool enabled;
    };

    Equalizer();
    ~Equalizer() override;

    void process(AudioBuffer& buffer) override;

    void setBandCount(int count);
    int getBandCount() const;

    void setBand(int index, const Band& band);
    Band getBand(int index) const;

    void setBandGain(int index, float gain);
    float getBandGain(int index) const;

    void setBandFrequency(int index, float frequency);
    float getBandFrequency(int index) const;

    void setBandQ(int index, float Q);
    float getBandQ(int index) const;

private:
    std::vector<Band> bands_;
};

class Compressor : public AudioEffect {
public:
    Compressor();
    ~Compressor() override;

    void process(AudioBuffer& buffer) override;

    void setThreshold(float threshold);
    float getThreshold() const;

    void setRatio(float ratio);
    float getRatio() const;

    void setAttack(float attack);
    float getAttack() const;

    void setRelease(float release);
    float getRelease() const;

    void setKnee(float knee);
    float getKnee() const;

    void setMakeupGain(float gain);
    float getMakeupGain() const;

private:
    float threshold_;
    float ratio_;
    float attack_;
    float release_;
    float knee_;
    float makeupGain_;

    float envelope_;
};

class Reverb : public AudioEffect {
public:
    Reverb();
    ~Reverb() override;

    void process(AudioBuffer& buffer) override;

    void setRoomSize(float size);
    float getRoomSize() const;

    void setDamping(float damping);
    float getDamping() const;

    void setWetLevel(float level);
    float getWetLevel() const;

    void setDryLevel(float level);
    float getDryLevel() const;

    void setWidth(float width);
    float getWidth() const;

private:
    float roomSize_;
    float damping_;
    float wetLevel_;
    float dryLevel_;
    float width_;

    std::vector<std::vector<float>> delayLines_;
    std::vector<float> delayTimes_;
    std::vector<float> feedbackGains_;
};

class Delay : public AudioEffect {
public:
    Delay();
    ~Delay() override;

    void process(AudioBuffer& buffer) override;

    void setDelayTime(float time);
    float getDelayTime() const;

    void setFeedback(float feedback);
    float getFeedback() const;

    void setMix(float mix);
    float getMix() const;

private:
    float delayTime_;
    float feedback_;
    std::vector<float> delayBuffer_;
    size_t writeIndex_;
};

class Chorus : public AudioEffect {
public:
    Chorus();
    ~Chorus() override;

    void process(AudioBuffer& buffer) override;

    void setRate(float rate);
    float getRate() const;

    void setDepth(float depth);
    float getDepth() const;

    void setMix(float mix);
    float getMix() const;

private:
    float rate_;
    float depth_;
    float phase_;

    std::vector<float> delayBuffer_;
    size_t writeIndex_;
};

class Distortion : public AudioEffect {
public:
    enum class Type {
        HardClip,
        SoftClip,
        Overdrive,
        Fuzz
    };

    Distortion();
    ~Distortion() override;

    void process(AudioBuffer& buffer) override;

    void setType(Type type);
    Type getType() const;

    void setDrive(float drive);
    float getDrive() const;

    void setTone(float tone);
    float getTone() const;

private:
    Type type_;
    float drive_;
    float tone_;
};

class Filter : public AudioEffect {
public:
    enum class Type {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass,
        Peaking,
        LowShelf,
        HighShelf
    };

    Filter();
    ~Filter() override;

    void process(AudioBuffer& buffer) override;

    void setType(Type type);
    Type getType() const;

    void setFrequency(float frequency);
    float getFrequency() const;

    void setQ(float Q);
    float getQ() const;

    void setGain(float gain);
    float getGain() const;

private:
    Type type_;
    float frequency_;
    float Q_;
    float gain_;

    std::vector<float> state1_;
    std::vector<float> state2_;
};

class EffectChain {
public:
    EffectChain();
    ~EffectChain();

    void addEffect(std::shared_ptr<AudioEffect> effect);
    void removeEffect(AudioEffect* effect);
    void clearEffects();

    void process(AudioBuffer& buffer);

    void setEnabled(bool enabled);
    bool isEnabled() const;

private:
    std::vector<std::shared_ptr<AudioEffect>> effects_;
    bool enabled_;
};

}
}