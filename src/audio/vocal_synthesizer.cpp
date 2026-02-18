#include "vocal_synthesizer.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace v3d {
namespace audio {

VocalSynthesizer::VocalSynthesizer()
    : tempo_(120.0f)
    , timeSignatureNumerator_(4)
    , timeSignatureDenominator_(4) {
}

VocalSynthesizer::~VocalSynthesizer() {
    shutdown();
}

bool VocalSynthesizer::initialize() {
    return true;
}

void VocalSynthesizer::shutdown() {
    clearNotes();
}

void VocalSynthesizer::setVoicebank(const std::string& path) {
    voicebank_ = path;
}

std::string VocalSynthesizer::getVoicebank() const {
    return voicebank_;
}

void VocalSynthesizer::addNote(const Note& note) {
    notes_.push_back(note);
}

void VocalSynthesizer::removeNote(int index) {
    if (index >= 0 && index < static_cast<int>(notes_.size())) {
        notes_.erase(notes_.begin() + index);
    }
}

void VocalSynthesizer::clearNotes() {
    notes_.clear();
}

const std::vector<Note>& VocalSynthesizer::getNotes() const {
    return notes_;
}

std::vector<Note>& VocalSynthesizer::getNotes() {
    return notes_;
}

void VocalSynthesizer::setSynthParams(const SynthParams& params) {
    synthParams_ = params;
}

SynthParams VocalSynthesizer::getSynthParams() const {
    return synthParams_;
}

AudioBuffer VocalSynthesizer::synthesize(float sampleRate) {
    if (notes_.empty()) {
        AudioSpec spec;
        spec.sampleRate = static_cast<int>(sampleRate);
        spec.format = AudioFormat::Float32;
        spec.channels = AudioChannelLayout::Stereo;
        return AudioBuffer(spec, 0);
    }

    float totalDuration = 0.0f;
    for (const auto& note : notes_) {
        totalDuration = std::max(totalDuration, note.startTime + note.duration);
    }

    AudioSpec spec;
    spec.sampleRate = static_cast<int>(sampleRate);
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Stereo;

    AudioBuffer result(spec, static_cast<size_t>(totalDuration * sampleRate));
    result.clear();

    float progress = 0.0f;
    float progressStep = 1.0f / notes_.size();

    for (const auto& note : notes_) {
        AudioBuffer noteBuffer = synthesizePhoneme(note.phonemes.empty() ? Phoneme::A : note.phonemes[0],
                                                note.duration,
                                                440.0f * std::pow(2.0f, (note.midiNote - 69) / 12.0f),
                                                synthParams_);

        size_t startFrame = static_cast<size_t>(note.startTime * sampleRate);
        size_t noteFrames = std::min(noteBuffer.getFrameCount(), result.getFrameCount() - startFrame);

        if (noteFrames > 0) {
            float* resultSamples = reinterpret_cast<float*>(result.getData());
            const float* noteSamples = reinterpret_cast<const float*>(noteBuffer.getData());

            for (size_t i = 0; i < noteFrames; ++i) {
                resultSamples[startFrame + i] += noteSamples[i];
            }
        }

        progress += progressStep;
        if (onProgress_) {
            onProgress_(progress);
        }
    }

    if (onComplete_) {
        onComplete_();
    }

    return result;
}

AudioBuffer VocalSynthesizer::synthesizeRange(float startTime, float endTime, float sampleRate) {
    AudioBuffer fullBuffer = synthesize(sampleRate);

    size_t startFrame = static_cast<size_t>(startTime * sampleRate);
    size_t endFrame = static_cast<size_t>(endTime * sampleRate);
    size_t frameCount = endFrame - startFrame;

    AudioSpec spec = fullBuffer.getSpec();
    AudioBuffer result(spec, frameCount);

    const float* srcSamples = reinterpret_cast<const float*>(fullBuffer.getData());
    float* dstSamples = reinterpret_cast<float*>(result.getData());

    std::memcpy(dstSamples, srcSamples + startFrame, frameCount * spec.getBytesPerFrame());

    return result;
}

void VocalSynthesizer::setTempo(float tempo) {
    tempo_ = std::max(1.0f, tempo);
}

float VocalSynthesizer::getTempo() const {
    return tempo_;
}

void VocalSynthesizer::setTimeSignature(int numerator, int denominator) {
    timeSignatureNumerator_ = std::max(1, numerator);
    timeSignatureDenominator_ = std::max(1, denominator);
}

void VocalSynthesizer::getTimeSignature(int& numerator, int& denominator) const {
    numerator = timeSignatureNumerator_;
    denominator = timeSignatureDenominator_;
}

void VocalSynthesizer::applyPitchBend(int noteIndex, const std::vector<float>& pitchPoints) {
    if (noteIndex >= 0 && noteIndex < static_cast<int>(notes_.size())) {
        notes_[noteIndex].pitchBend = pitchPoints.empty() ? 0.0f : pitchPoints[0];
    }
}

void VocalSynthesizer::applyVibrato(int noteIndex, float depth, float rate) {
    if (noteIndex >= 0 && noteIndex < static_cast<int>(notes_.size())) {
        notes_[noteIndex].vibratoDepth = depth;
        notes_[noteIndex].vibratoRate = rate;
    }
}

void VocalSynthesizer::setLyrics(const std::string& lyrics) {
    for (auto& note : notes_) {
        note.lyrics = lyrics;
    }
}

std::string VocalSynthesizer::getLyrics() const {
    if (notes_.empty()) {
        return "";
    }
    return notes_[0].lyrics;
}

void VocalSynthesizer::convertLyricsToPhonemes() {
    for (auto& note : notes_) {
        note.phonemes = lyricsToPhonemes(note.lyrics);
    }
}

void VocalSynthesizer::setExpression(const std::string& name, float value) {
    expressions_[name] = value;
}

float VocalSynthesizer::getExpression(const std::string& name) const {
    auto it = expressions_.find(name);
    return it != expressions_.end() ? it->second : 0.0f;
}

void VocalSynthesizer::loadProject(const std::string& filePath) {
}

void VocalSynthesizer::saveProject(const std::string& filePath) const {
}

void VocalSynthesizer::setOnProgress(std::function<void(float)> callback) {
    onProgress_ = callback;
}

void VocalSynthesizer::setOnComplete(std::function<void()> callback) {
    onComplete_ = callback;
}

std::vector<Phoneme> VocalSynthesizer::lyricsToPhonemes(const std::string& lyrics) {
    std::vector<Phoneme> phonemes;

    std::string lowerLyrics = lyrics;
    std::transform(lowerLyrics.begin(), lowerLyrics.end(), lowerLyrics.begin(), ::tolower);

    for (char c : lowerLyrics) {
        switch (c) {
            case 'a': phonemes.push_back(Phoneme::A); break;
            case 'i': phonemes.push_back(Phoneme::I); break;
            case 'u': phonemes.push_back(Phoneme::U); break;
            case 'e': phonemes.push_back(Phoneme::E); break;
            case 'o': phonemes.push_back(Phoneme::O); break;
            case 'k': phonemes.push_back(Phoneme::Ka); break;
            case 's': phonemes.push_back(Phoneme::Sa); break;
            case 't': phonemes.push_back(Phoneme::Ta); break;
            case 'n': phonemes.push_back(Phoneme::Na); break;
            case 'h': phonemes.push_back(Phoneme::Ha); break;
            case 'm': phonemes.push_back(Phoneme::Ma); break;
            case 'y': phonemes.push_back(Phoneme::Ya); break;
            case 'r': phonemes.push_back(Phoneme::Ra); break;
            case 'w': phonemes.push_back(Phoneme::Wa); break;
            case ' ': phonemes.push_back(Phoneme::VowelPause); break;
            default: break;
        }
    }

    if (phonemes.empty()) {
        phonemes.push_back(Phoneme::A);
    }

    return phonemes;
}

AudioBuffer VocalSynthesizer::synthesizePhoneme(Phoneme phoneme, float duration, float frequency, const SynthParams& params) {
    AudioSpec spec;
    spec.sampleRate = 44100;
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Mono;

    size_t frameCount = static_cast<size_t>(duration * spec.sampleRate);
    AudioBuffer buffer(spec, frameCount);

    float* samples = reinterpret_cast<float*>(buffer.getData());

    float phase = 0.0f;
    float phaseIncrement = 2.0f * 3.14159265359f * frequency / spec.sampleRate;

    float amplitude = params.breathiness * 0.3f + 0.7f;
    float brightness = params.brightness;

    for (size_t i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / frameCount;

        float envelope = 1.0f;
        if (t < 0.1f) {
            envelope = t / 0.1f;
        } else if (t > 0.9f) {
            envelope = (1.0f - t) / 0.1f;
        }

        float waveform = std::sin(phase);

        switch (phoneme) {
            case Phoneme::A:
                waveform += 0.3f * std::sin(phase * 2.0f);
                break;
            case Phoneme::I:
                waveform += 0.5f * std::sin(phase * 3.0f);
                break;
            case Phoneme::U:
                waveform += 0.2f * std::sin(phase * 1.5f);
                break;
            case Phoneme::E:
                waveform += 0.4f * std::sin(phase * 2.5f);
                break;
            case Phoneme::O:
                waveform += 0.3f * std::sin(phase * 1.8f);
                break;
            default:
                break;
        }

        float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f * params.breathiness;

        samples[i] = (waveform + noise) * envelope * amplitude;

        phase += phaseIncrement;
        if (phase > 2.0f * 3.14159265359f) {
            phase -= 2.0f * 3.14159265359f;
        }
    }

    return buffer;
}

float VocalSynthesizer::getPhonemeDuration(Phoneme phoneme, float noteDuration) {
    return noteDuration;
}

Voicebank::Voicebank() {
}

Voicebank::~Voicebank() {
}

bool Voicebank::load(const std::string& path) {
    path_ = path;
    return true;
}

void Voicebank::save(const std::string& path) const {
}

std::string Voicebank::getName() const {
    return name_;
}

void Voicebank::setName(const std::string& name) {
    name_ = name;
}

AudioBuffer Voicebank::getPhonemeSample(Phoneme phoneme, float frequency) const {
    AudioSpec spec;
    spec.sampleRate = 44100;
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Mono;

    return AudioBuffer(spec, 0);
}

void Voicebank::addPhonemeSample(Phoneme phoneme, float frequency, const AudioBuffer& sample) {
}

std::vector<float> Voicebank::getSupportedFrequencies() const {
    return {440.0f, 880.0f, 1760.0f};
}

SingingSynth::SingingSynth()
    : breathiness_(0.0f)
    , brightness_(0.5f)
    , opening_(0.5f)
    , gender_(0.0f) {
}

SingingSynth::~SingingSynth() {
}

bool SingingSynth::initialize() {
    return true;
}

void SingingSynth::shutdown() {
}

void SingingSynth::setVoicebank(std::shared_ptr<Voicebank> voicebank) {
    voicebank_ = voicebank;
}

std::shared_ptr<Voicebank> SingingSynth::getVoicebank() const {
    return voicebank_;
}

AudioBuffer SingingSynth::synthesize(const std::vector<Note>& notes, const SynthParams& params, float sampleRate) {
    if (notes.empty()) {
        AudioSpec spec;
        spec.sampleRate = static_cast<int>(sampleRate);
        spec.format = AudioFormat::Float32;
        spec.channels = AudioChannelLayout::Stereo;
        return AudioBuffer(spec, 0);
    }

    float totalDuration = 0.0f;
    for (const auto& note : notes) {
        totalDuration = std::max(totalDuration, note.startTime + note.duration);
    }

    AudioSpec spec;
    spec.sampleRate = static_cast<int>(sampleRate);
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Stereo;

    AudioBuffer result(spec, static_cast<size_t>(totalDuration * sampleRate));
    result.clear();

    for (const auto& note : notes) {
        AudioBuffer noteBuffer = synthesizeNote(note, params, sampleRate);

        size_t startFrame = static_cast<size_t>(note.startTime * sampleRate);
        size_t noteFrames = std::min(noteBuffer.getFrameCount(), result.getFrameCount() - startFrame);

        if (noteFrames > 0) {
            float* resultSamples = reinterpret_cast<float*>(result.getData());
            const float* noteSamples = reinterpret_cast<const float*>(noteBuffer.getData());

            for (size_t i = 0; i < noteFrames; ++i) {
                resultSamples[startFrame + i] += noteSamples[i];
            }
        }
    }

    return result;
}

void SingingSynth::setBreathiness(float value) {
    breathiness_ = std::clamp(value, 0.0f, 1.0f);
}

float SingingSynth::getBreathiness() const {
    return breathiness_;
}

void SingingSynth::setBrightness(float value) {
    brightness_ = std::clamp(value, 0.0f, 1.0f);
}

float SingingSynth::getBrightness() const {
    return brightness_;
}

void SingingSynth::setOpening(float value) {
    opening_ = std::clamp(value, 0.0f, 1.0f);
}

float SingingSynth::getOpening() const {
    return opening_;
}

void SingingSynth::setGender(float value) {
    gender_ = std::clamp(value, -1.0f, 1.0f);
}

float SingingSynth::getGender() const {
    return gender_;
}

AudioBuffer SingingSynth::synthesizeNote(const Note& note, const SynthParams& params, float sampleRate) {
    AudioSpec spec;
    spec.sampleRate = static_cast<int>(sampleRate);
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Mono;

    size_t frameCount = static_cast<size_t>(note.duration * sampleRate);
    AudioBuffer buffer(spec, frameCount);

    float* samples = reinterpret_cast<float*>(buffer.getData());

    float frequency = 440.0f * std::pow(2.0f, (note.midiNote - 69) / 12.0f);
    float phase = 0.0f;
    float phaseIncrement = 2.0f * 3.14159265359f * frequency / sampleRate;

    float amplitude = note.velocity / 127.0f;

    for (size_t i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / frameCount;

        float envelope = 1.0f;
        if (t < 0.05f) {
            envelope = t / 0.05f;
        } else if (t > 0.95f) {
            envelope = (1.0f - t) / 0.05f;
        }

        float waveform = std::sin(phase);
        waveform += 0.5f * std::sin(phase * 2.0f);
        waveform += 0.25f * std::sin(phase * 3.0f);

        samples[i] = waveform * envelope * amplitude;

        phase += phaseIncrement;
        if (phase > 2.0f * 3.14159265359f) {
            phase -= 2.0f * 3.14159265359f;
        }
    }

    buffer = applyFormants(buffer, frequency, params);
    buffer = applyVibrato(buffer, note.duration, note.vibratoDepth, note.vibratoRate, sampleRate);

    return buffer;
}

AudioBuffer SingingSynth::applyFormants(const AudioBuffer& input, float frequency, const SynthParams& params) {
    AudioBuffer output = input.clone();

    return output;
}

AudioBuffer SingingSynth::applyVibrato(const AudioBuffer& input, float duration, float depth, float rate, float sampleRate) {
    if (depth < 0.001f || rate < 0.001f) {
        return input.clone();
    }

    AudioBuffer output = input.clone();

    if (output.getSpec().format != AudioFormat::Float32) {
        return output;
    }

    float* samples = reinterpret_cast<float*>(output.getData());
    size_t frameCount = output.getFrameCount();

    for (size_t i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float vibrato = std::sin(2.0f * 3.14159265359f * rate * t) * depth;
        samples[i] *= (1.0f + vibrato);
    }

    return output;
}

AudioBuffer SingingSynth::applyPitchBend(const AudioBuffer& input, const std::vector<float>& pitchPoints, float duration, float sampleRate) {
    return input.clone();
}

}
}