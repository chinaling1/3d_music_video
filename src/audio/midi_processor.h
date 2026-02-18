#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cstdint>
#include <memory>

namespace v3d {
namespace audio {

enum class MIDIEventType {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyphonicPressure = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelPressure = 0xD0,
    PitchBend = 0xE0,
    SystemExclusive = 0xF0,
    TimeCode = 0xF1,
    SongPosition = 0xF2,
    SongSelect = 0xF3,
    TuneRequest = 0xF6,
    EndOfExclusive = 0xF7,
    TimingClock = 0xF8,
    Start = 0xFA,
    Continue = 0xFB,
    Stop = 0xFC,
    ActiveSensing = 0xFE,
    Reset = 0xFF
};

struct MIDIMessage {
    MIDIEventType type;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;
    std::vector<uint8_t> sysexData;
    double timestamp;

    MIDIMessage()
        : type(MIDIEventType::NoteOff)
        , channel(0)
        , data1(0)
        , data2(0)
        , timestamp(0.0) {
    }

    bool isNoteMessage() const {
        return type == MIDIEventType::NoteOn || type == MIDIEventType::NoteOff;
    }

    bool isControlMessage() const {
        return type == MIDIEventType::ControlChange;
    }

    int getNoteNumber() const {
        return data1;
    }

    int getVelocity() const {
        return data2;
    }

    int getControllerNumber() const {
        return data1;
    }

    int getControllerValue() const {
        return data2;
    }

    int getProgramNumber() const {
        return data1;
    }

    int getPitchBendValue() const {
        return (data2 << 7) | data1;
    }
};

struct MIDITrack {
    std::string name;
    int channel;
    std::vector<MIDIMessage> messages;
};

class MIDIFile {
public:
    MIDIFile();
    ~MIDIFile();

    bool load(const std::string& filePath);
    bool save(const std::string& filePath) const;

    int getFormat() const;
    int getTicksPerQuarterNote() const;
    int getTrackCount() const;

    const std::vector<MIDITrack>& getTracks() const;
    std::vector<MIDITrack>& getTracks();

    void addTrack(const MIDITrack& track);
    void removeTrack(int index);
    void clearTracks();

    float getDuration() const;
    float getTempo() const;
    void setTempo(float tempo);

    int getTimeSignatureNumerator() const;
    int getTimeSignatureDenominator() const;
    void setTimeSignature(int numerator, int denominator);

private:
    int format_;
    int ticksPerQuarterNote_;
    std::vector<MIDITrack> tracks_;
    float tempo_;
    int timeSignatureNumerator_;
    int timeSignatureDenominator_;
};

class MIDIProcessor {
public:
    MIDIProcessor();
    ~MIDIProcessor();

    void processMessage(const MIDIMessage& message);

    void setNoteOnCallback(std::function<void(int, int, int)> callback);
    void setNoteOffCallback(std::function<void(int, int)> callback);
    void setControlChangeCallback(std::function<void(int, int, int)> callback);
    void setPitchBendCallback(std::function<void(int, int)> callback);
    void setProgramChangeCallback(std::function<void(int, int)> callback);

    void reset();

    void setChannel(int channel);
    int getChannel() const;

    void setTranspose(int semitones);
    int getTranspose() const;

    void setVelocityCurve(const std::vector<float>& curve);
    std::vector<float> getVelocityCurve() const;

    void enableVelocitySensitivity(bool enable);
    bool isVelocitySensitivityEnabled() const;

    void enablePitchBend(bool enable);
    bool isPitchBendEnabled() const;

    void setPitchBendRange(int semitones);
    int getPitchBendRange() const;

    void setModulationWheel(int value);
    int getModulationWheel() const;

    void setSustainPedal(bool pressed);
    bool isSustainPedalPressed() const;

    void setSostenutoPedal(bool pressed);
    bool isSostenutoPedalPressed() const;

    void setSoftPedal(bool pressed);
    bool isSoftPedalPressed() const;

private:
    int channel_;
    int transpose_;
    std::vector<float> velocityCurve_;
    bool velocitySensitivity_;
    bool pitchBendEnabled_;
    int pitchBendRange_;
    int pitchBendValue_;
    int modulationWheel_;
    bool sustainPedal_;
    bool sostenutoPedal_;
    bool softPedal_;

    std::function<void(int, int, int)> noteOnCallback_;
    std::function<void(int, int)> noteOffCallback_;
    std::function<void(int, int, int)> controlChangeCallback_;
    std::function<void(int, int)> pitchBendCallback_;
    std::function<void(int, int)> programChangeCallback_;
};

class MIDISequencer {
public:
    MIDISequencer();
    ~MIDISequencer();

    void setMIDIFile(std::shared_ptr<MIDIFile> file);
    std::shared_ptr<MIDIFile> getMIDIFile() const;

    void play();
    void pause();
    void stop();

    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    void setPosition(float time);
    float getPosition() const;

    void setTempo(float tempo);
    float getTempo() const;

    void setLooping(bool looping);
    bool isLooping() const;

    void setLoopStart(float time);
    float getLoopStart() const;

    void setLoopEnd(float time);
    float getLoopEnd() const;

    float getDuration() const;

    void update(float deltaTime);

    void setOnMessageCallback(std::function<void(const MIDIMessage&)> callback);

    void setOnNoteOnCallback(std::function<void(int, int, int)> callback);
    void setOnNoteOffCallback(std::function<void(int, int)> callback);

    void setOnEndCallback(std::function<void()> callback);

private:
    std::shared_ptr<MIDIFile> midiFile_;
    bool playing_;
    bool paused_;
    bool looping_;
    float currentTime_;
    float tempo_;
    float loopStart_;
    float loopEnd_;

    std::function<void(const MIDIMessage&)> onMessageCallback_;
    std::function<void(int, int, int)> onNoteOnCallback_;
    std::function<void(int, int)> onNoteOffCallback_;
    std::function<void()> onEndCallback_;
};

}
}