#pragma once

#include "audio_buffer.h"
#include "audio_effect.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace v3d {
namespace audio {

enum class Phoneme {
    A,
    I,
    U,
    E,
    O,
    Ka,
    Ki,
    Ku,
    Ke,
    Ko,
    Sa,
    Shi,
    Su,
    Se,
    So,
    Ta,
    Chi,
    Tsu,
    Te,
    To,
    Na,
    Ni,
    Nu,
    Ne,
    No,
    Ha,
    Hi,
    Fu,
    He,
    Ho,
    Ma,
    Mi,
    Mu,
    Me,
    Mo,
    Ya,
    Yu,
    Yo,
    Ra,
    Ri,
    Ru,
    Re,
    Ro,
    Wa,
    Wo,
    N,
    VowelPause,
    ConsonantPause
};

struct PhonemeFreqHash {
    size_t operator()(const std::pair<Phoneme, float>& p) const {
        size_t h1 = std::hash<int>()(static_cast<int>(p.first));
        size_t h2 = std::hash<float>()(p.second);
        return h1 ^ (h2 << 1);
    }
};

struct Note {
    int midiNote;
    float startTime;
    float duration;
    float velocity;
    std::string lyrics;
    std::vector<Phoneme> phonemes;
    float pitchBend;
    float vibratoDepth;
    float vibratoRate;
};

struct SynthParams {
    std::string voicebank;
    float breathiness;
    float brightness;
    float opening;
    float gender;
    float growl;
    float falsetto;
    float tension;
    float voicing;
};

class VocalSynthesizer {
public:
    VocalSynthesizer();
    ~VocalSynthesizer();

    bool initialize();
    void shutdown();

    void setVoicebank(const std::string& path);
    std::string getVoicebank() const;

    void addNote(const Note& note);
    void removeNote(int index);
    void clearNotes();

    const std::vector<Note>& getNotes() const;
    std::vector<Note>& getNotes();

    void setSynthParams(const SynthParams& params);
    SynthParams getSynthParams() const;

    AudioBuffer synthesize(float sampleRate = 44100.0f);
    AudioBuffer synthesizeRange(float startTime, float endTime, float sampleRate = 44100.0f);

    void setTempo(float tempo);
    float getTempo() const;

    void setTimeSignature(int numerator, int denominator);
    void getTimeSignature(int& numerator, int& denominator) const;

    void applyPitchBend(int noteIndex, const std::vector<float>& pitchPoints);
    void applyVibrato(int noteIndex, float depth, float rate);

    void setLyrics(const std::string& lyrics);
    std::string getLyrics() const;

    void convertLyricsToPhonemes();

    void setExpression(const std::string& name, float value);
    float getExpression(const std::string& name) const;

    void loadProject(const std::string& filePath);
    void saveProject(const std::string& filePath) const;

    void setOnProgress(std::function<void(float)> callback);
    void setOnComplete(std::function<void()> callback);

private:
    std::vector<Phoneme> lyricsToPhonemes(const std::string& lyrics);
    AudioBuffer synthesizePhoneme(Phoneme phoneme, float duration, float frequency, const SynthParams& params);
    float getPhonemeDuration(Phoneme phoneme, float noteDuration);

    std::vector<Note> notes_;
    SynthParams synthParams_;
    std::string voicebank_;

    float tempo_;
    int timeSignatureNumerator_;
    int timeSignatureDenominator_;

    std::unordered_map<std::string, float> expressions_;

    std::function<void(float)> onProgress_;
    std::function<void()> onComplete_;
};

class Voicebank {
public:
    Voicebank();
    ~Voicebank();

    bool load(const std::string& path);
    void save(const std::string& path) const;

    std::string getName() const;
    void setName(const std::string& name);

    AudioBuffer getPhonemeSample(Phoneme phoneme, float frequency) const;

    void addPhonemeSample(Phoneme phoneme, float frequency, const AudioBuffer& sample);

    std::vector<float> getSupportedFrequencies() const;

private:
    std::string name_;
    std::string path_;
    std::unordered_map<std::pair<Phoneme, float>, AudioBuffer, PhonemeFreqHash> phonemeSamples_;
};

class SingingSynth {
public:
    SingingSynth();
    ~SingingSynth();

    bool initialize();
    void shutdown();

    void setVoicebank(std::shared_ptr<Voicebank> voicebank);
    std::shared_ptr<Voicebank> getVoicebank() const;

    AudioBuffer synthesize(const std::vector<Note>& notes, const SynthParams& params, float sampleRate = 44100.0f);

    void setBreathiness(float value);
    float getBreathiness() const;

    void setBrightness(float value);
    float getBrightness() const;

    void setOpening(float value);
    float getOpening() const;

    void setGender(float value);
    float getGender() const;

private:
    AudioBuffer synthesizeNote(const Note& note, const SynthParams& params, float sampleRate);
    AudioBuffer applyFormants(const AudioBuffer& input, float frequency, const SynthParams& params);
    AudioBuffer applyVibrato(const AudioBuffer& input, float duration, float depth, float rate, float sampleRate);
    AudioBuffer applyPitchBend(const AudioBuffer& input, const std::vector<float>& pitchPoints, float duration, float sampleRate);

    std::shared_ptr<Voicebank> voicebank_;
    float breathiness_;
    float brightness_;
    float opening_;
    float gender_;
};

}
}