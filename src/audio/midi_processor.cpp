#include "midi_processor.h"
#include <algorithm>
#include <fstream>

namespace v3d {
namespace audio {

MIDIFile::MIDIFile()
    : format_(0)
    , ticksPerQuarterNote_(480)
    , tempo_(120.0f)
    , timeSignatureNumerator_(4)
    , timeSignatureDenominator_(4) {
}

MIDIFile::~MIDIFile() {
}

bool MIDIFile::load(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    char header[4];
    file.read(header, 4);

    if (header[0] != 'M' || header[1] != 'T' || header[2] != 'h' || header[3] != 'd') {
        return false;
    }

    uint32_t headerLength;
    file.read(reinterpret_cast<char*>(&headerLength), 4);

    uint16_t format;
    file.read(reinterpret_cast<char*>(&format), 2);
    format_ = ((format & 0xFF) << 8) | ((format >> 8) & 0xFF);

    uint16_t trackCount;
    file.read(reinterpret_cast<char*>(&trackCount), 2);

    uint16_t division;
    file.read(reinterpret_cast<char*>(&division), 2);
    ticksPerQuarterNote_ = ((division & 0xFF) << 8) | ((division >> 8) & 0xFF);

    tracks_.clear();
    tracks_.reserve(trackCount);

    for (int i = 0; i < trackCount; ++i) {
        MIDITrack track;

        char trackHeader[4];
        file.read(trackHeader, 4);

        if (trackHeader[0] != 'M' || trackHeader[1] != 'T' || trackHeader[2] != 'r' || trackHeader[3] != 'k') {
            break;
        }

        uint32_t trackLength;
        file.read(reinterpret_cast<char*>(&trackLength), 4);

        size_t trackEnd = static_cast<size_t>(file.tellg()) + static_cast<size_t>(trackLength);

        while (file.tellg() < static_cast<std::streampos>(trackEnd)) {
            MIDIMessage message;

            uint32_t deltaTime = 0;
            uint8_t byte;
            do {
                file.read(reinterpret_cast<char*>(&byte), 1);
                deltaTime = (deltaTime << 7) | (byte & 0x7F);
            } while (byte & 0x80);

            message.timestamp = deltaTime;

            file.read(reinterpret_cast<char*>(&byte), 1);
            message.type = static_cast<MIDIEventType>(byte & 0xF0);
            message.channel = byte & 0x0F;

            if (message.type == MIDIEventType::NoteOn || message.type == MIDIEventType::NoteOff ||
                message.type == MIDIEventType::PolyphonicPressure || message.type == MIDIEventType::ControlChange ||
                message.type == MIDIEventType::PitchBend) {

                file.read(reinterpret_cast<char*>(&message.data1), 1);
                file.read(reinterpret_cast<char*>(&message.data2), 1);
            } else if (message.type == MIDIEventType::ProgramChange || message.type == MIDIEventType::ChannelPressure) {
                file.read(reinterpret_cast<char*>(&message.data1), 1);
            } else if (message.type == MIDIEventType::SystemExclusive) {
                uint8_t length;
                file.read(reinterpret_cast<char*>(&length), 1);
                message.sysexData.resize(length);
                file.read(reinterpret_cast<char*>(message.sysexData.data()), length);
            }

            track.messages.push_back(message);
        }

        tracks_.push_back(track);
    }

    return true;
}

bool MIDIFile::save(const std::string& filePath) const {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write("MThd", 4);

    uint32_t headerLength = 6;
    file.write(reinterpret_cast<const char*>(&headerLength), 4);

    uint16_t format = static_cast<uint16_t>(format_);
    file.write(reinterpret_cast<const char*>(&format), 2);

    uint16_t trackCount = static_cast<uint16_t>(tracks_.size());
    file.write(reinterpret_cast<const char*>(&trackCount), 2);

    uint16_t division = static_cast<uint16_t>(ticksPerQuarterNote_);
    file.write(reinterpret_cast<const char*>(&division), 2);

    for (const auto& track : tracks_) {
        file.write("MTrk", 4);

        std::vector<uint8_t> trackData;

        for (const auto& message : track.messages) {
            uint32_t deltaTime = static_cast<uint32_t>(message.timestamp);

            std::vector<uint8_t> deltaTimeBytes;
            do {
                uint8_t byte = deltaTime & 0x7F;
                deltaTime >>= 7;
                if (deltaTime > 0) {
                    byte |= 0x80;
                }
                deltaTimeBytes.push_back(byte);
            } while (deltaTime > 0);

            trackData.insert(trackData.end(), deltaTimeBytes.rbegin(), deltaTimeBytes.rend());

            uint8_t statusByte = static_cast<uint8_t>(message.type) | message.channel;
            trackData.push_back(statusByte);

            if (message.type == MIDIEventType::NoteOn || message.type == MIDIEventType::NoteOff ||
                message.type == MIDIEventType::PolyphonicPressure || message.type == MIDIEventType::ControlChange ||
                message.type == MIDIEventType::PitchBend) {

                trackData.push_back(message.data1);
                trackData.push_back(message.data2);
            } else if (message.type == MIDIEventType::ProgramChange || message.type == MIDIEventType::ChannelPressure) {
                trackData.push_back(message.data1);
            } else if (message.type == MIDIEventType::SystemExclusive) {
                trackData.push_back(static_cast<uint8_t>(message.sysexData.size()));
                trackData.insert(trackData.end(), message.sysexData.begin(), message.sysexData.end());
            }
        }

        uint32_t trackLength = static_cast<uint32_t>(trackData.size());
        file.write(reinterpret_cast<const char*>(&trackLength), 4);
        file.write(reinterpret_cast<const char*>(trackData.data()), trackData.size());
    }

    return true;
}

int MIDIFile::getFormat() const {
    return format_;
}

int MIDIFile::getTicksPerQuarterNote() const {
    return ticksPerQuarterNote_;
}

int MIDIFile::getTrackCount() const {
    return static_cast<int>(tracks_.size());
}

const std::vector<MIDITrack>& MIDIFile::getTracks() const {
    return tracks_;
}

std::vector<MIDITrack>& MIDIFile::getTracks() {
    return tracks_;
}

void MIDIFile::addTrack(const MIDITrack& track) {
    tracks_.push_back(track);
}

void MIDIFile::removeTrack(int index) {
    if (index >= 0 && index < static_cast<int>(tracks_.size())) {
        tracks_.erase(tracks_.begin() + index);
    }
}

void MIDIFile::clearTracks() {
    tracks_.clear();
}

float MIDIFile::getDuration() const {
    return 0.0f;
}

float MIDIFile::getTempo() const {
    return tempo_;
}

void MIDIFile::setTempo(float tempo) {
    tempo_ = std::max(1.0f, tempo);
}

int MIDIFile::getTimeSignatureNumerator() const {
    return timeSignatureNumerator_;
}

int MIDIFile::getTimeSignatureDenominator() const {
    return timeSignatureDenominator_;
}

void MIDIFile::setTimeSignature(int numerator, int denominator) {
    timeSignatureNumerator_ = std::max(1, numerator);
    timeSignatureDenominator_ = std::max(1, denominator);
}

MIDIProcessor::MIDIProcessor()
    : channel_(0)
    , transpose_(0)
    , velocitySensitivity_(true)
    , pitchBendEnabled_(true)
    , pitchBendRange_(2)
    , pitchBendValue_(8192)
    , modulationWheel_(0)
    , sustainPedal_(false)
    , sostenutoPedal_(false)
    , softPedal_(false) {
}

MIDIProcessor::~MIDIProcessor() {
}

void MIDIProcessor::processMessage(const MIDIMessage& message) {
    if (message.channel != channel_) {
        return;
    }

    switch (message.type) {
        case MIDIEventType::NoteOn:
            if (message.getVelocity() > 0) {
                int note = message.getNoteNumber() + transpose_;
                int velocity = message.getVelocity();

                if (velocitySensitivity_ && !velocityCurve_.empty()) {
                    int curveIndex = std::min(velocity, static_cast<int>(velocityCurve_.size()) - 1);
                    velocity = static_cast<int>(velocity * velocityCurve_[curveIndex]);
                }

                if (noteOnCallback_) {
                    noteOnCallback_(message.channel, note, velocity);
                }
            } else {
                if (noteOffCallback_) {
                    noteOffCallback_(message.channel, message.getNoteNumber());
                }
            }
            break;

        case MIDIEventType::NoteOff:
            if (noteOffCallback_) {
                noteOffCallback_(message.channel, message.getNoteNumber());
            }
            break;

        case MIDIEventType::ControlChange:
            if (controlChangeCallback_) {
                controlChangeCallback_(message.channel, message.getControllerNumber(), message.getControllerValue());
            }

            switch (message.getControllerNumber()) {
                case 1:
                    modulationWheel_ = message.getControllerValue();
                    break;
                case 64:
                    sustainPedal_ = message.getControllerValue() >= 64;
                    break;
                case 66:
                    sostenutoPedal_ = message.getControllerValue() >= 64;
                    break;
                case 67:
                    softPedal_ = message.getControllerValue() >= 64;
                    break;
            }
            break;

        case MIDIEventType::PitchBend:
            pitchBendValue_ = message.getPitchBendValue();
            if (pitchBendEnabled_ && pitchBendCallback_) {
                pitchBendCallback_(message.channel, pitchBendValue_);
            }
            break;

        case MIDIEventType::ProgramChange:
            if (programChangeCallback_) {
                programChangeCallback_(message.channel, message.getProgramNumber());
            }
            break;

        default:
            break;
    }
}

void MIDIProcessor::setNoteOnCallback(std::function<void(int, int, int)> callback) {
    noteOnCallback_ = callback;
}

void MIDIProcessor::setNoteOffCallback(std::function<void(int, int)> callback) {
    noteOffCallback_ = callback;
}

void MIDIProcessor::setControlChangeCallback(std::function<void(int, int, int)> callback) {
    controlChangeCallback_ = callback;
}

void MIDIProcessor::setPitchBendCallback(std::function<void(int, int)> callback) {
    pitchBendCallback_ = callback;
}

void MIDIProcessor::setProgramChangeCallback(std::function<void(int, int)> callback) {
    programChangeCallback_ = callback;
}

void MIDIProcessor::reset() {
    pitchBendValue_ = 8192;
    modulationWheel_ = 0;
    sustainPedal_ = false;
    sostenutoPedal_ = false;
    softPedal_ = false;
}

void MIDIProcessor::setChannel(int channel) {
    channel_ = std::clamp(channel, 0, 15);
}

int MIDIProcessor::getChannel() const {
    return channel_;
}

void MIDIProcessor::setTranspose(int semitones) {
    transpose_ = std::clamp(semitones, -24, 24);
}

int MIDIProcessor::getTranspose() const {
    return transpose_;
}

void MIDIProcessor::setVelocityCurve(const std::vector<float>& curve) {
    velocityCurve_ = curve;
}

std::vector<float> MIDIProcessor::getVelocityCurve() const {
    return velocityCurve_;
}

void MIDIProcessor::enableVelocitySensitivity(bool enable) {
    velocitySensitivity_ = enable;
}

bool MIDIProcessor::isVelocitySensitivityEnabled() const {
    return velocitySensitivity_;
}

void MIDIProcessor::enablePitchBend(bool enable) {
    pitchBendEnabled_ = enable;
}

bool MIDIProcessor::isPitchBendEnabled() const {
    return pitchBendEnabled_;
}

void MIDIProcessor::setPitchBendRange(int semitones) {
    pitchBendRange_ = std::max(1, semitones);
}

int MIDIProcessor::getPitchBendRange() const {
    return pitchBendRange_;
}

void MIDIProcessor::setModulationWheel(int value) {
    modulationWheel_ = std::clamp(value, 0, 127);
}

int MIDIProcessor::getModulationWheel() const {
    return modulationWheel_;
}

void MIDIProcessor::setSustainPedal(bool pressed) {
    sustainPedal_ = pressed;
}

bool MIDIProcessor::isSustainPedalPressed() const {
    return sustainPedal_;
}

void MIDIProcessor::setSostenutoPedal(bool pressed) {
    sostenutoPedal_ = pressed;
}

bool MIDIProcessor::isSostenutoPedalPressed() const {
    return sostenutoPedal_;
}

void MIDIProcessor::setSoftPedal(bool pressed) {
    softPedal_ = pressed;
}

bool MIDIProcessor::isSoftPedalPressed() const {
    return softPedal_;
}

MIDISequencer::MIDISequencer()
    : playing_(false)
    , paused_(false)
    , looping_(false)
    , currentTime_(0.0f)
    , tempo_(120.0f)
    , loopStart_(0.0f)
    , loopEnd_(0.0f) {
}

MIDISequencer::~MIDISequencer() {
}

void MIDISequencer::setMIDIFile(std::shared_ptr<MIDIFile> file) {
    midiFile_ = file;
    if (file) {
        loopEnd_ = file->getDuration();
    }
}

std::shared_ptr<MIDIFile> MIDISequencer::getMIDIFile() const {
    return midiFile_;
}

void MIDISequencer::play() {
    playing_ = true;
    paused_ = false;
}

void MIDISequencer::pause() {
    paused_ = true;
}

void MIDISequencer::stop() {
    playing_ = false;
    paused_ = false;
    currentTime_ = 0.0f;
}

bool MIDISequencer::isPlaying() const {
    return playing_;
}

bool MIDISequencer::isPaused() const {
    return paused_;
}

bool MIDISequencer::isStopped() const {
    return !playing_;
}

void MIDISequencer::setPosition(float time) {
    currentTime_ = std::max(0.0f, time);
}

float MIDISequencer::getPosition() const {
    return currentTime_;
}

void MIDISequencer::setTempo(float tempo) {
    tempo_ = std::max(1.0f, tempo);
}

float MIDISequencer::getTempo() const {
    return tempo_;
}

void MIDISequencer::setLooping(bool looping) {
    looping_ = looping;
}

bool MIDISequencer::isLooping() const {
    return looping_;
}

void MIDISequencer::setLoopStart(float time) {
    loopStart_ = std::max(0.0f, time);
}

float MIDISequencer::getLoopStart() const {
    return loopStart_;
}

void MIDISequencer::setLoopEnd(float time) {
    loopEnd_ = time;
}

float MIDISequencer::getLoopEnd() const {
    return loopEnd_;
}

float MIDISequencer::getDuration() const {
    if (!midiFile_) {
        return 0.0f;
    }
    return midiFile_->getDuration();
}

void MIDISequencer::update(float deltaTime) {
    if (!playing_ || paused_ || !midiFile_) {
        return;
    }

    float tempo = midiFile_->getTempo();
    float secondsPerBeat = 60.0f / tempo;
    float secondsPerTick = secondsPerBeat / midiFile_->getTicksPerQuarterNote();

    currentTime_ += deltaTime;

    if (looping_ && currentTime_ >= loopEnd_) {
        currentTime_ = loopStart_;
    }

    if (currentTime_ >= getDuration()) {
        playing_ = false;
        if (onEndCallback_) {
            onEndCallback_();
        }
    }

    float currentTick = currentTime_ / secondsPerTick;

    for (const auto& track : midiFile_->getTracks()) {
        for (const auto& message : track.messages) {
            float messageTime = message.timestamp * secondsPerTick;

            if (std::abs(messageTime - currentTime_) < deltaTime) {
                if (onMessageCallback_) {
                    onMessageCallback_(message);
                }

                if (message.isNoteMessage()) {
                    if (message.type == MIDIEventType::NoteOn && message.getVelocity() > 0) {
                        if (onNoteOnCallback_) {
                            onNoteOnCallback_(message.channel, message.getNoteNumber(), message.getVelocity());
                        }
                    } else {
                        if (onNoteOffCallback_) {
                            onNoteOffCallback_(message.channel, message.getNoteNumber());
                        }
                    }
                }
            }
        }
    }
}

void MIDISequencer::setOnMessageCallback(std::function<void(const MIDIMessage&)> callback) {
    onMessageCallback_ = callback;
}

void MIDISequencer::setOnNoteOnCallback(std::function<void(int, int, int)> callback) {
    onNoteOnCallback_ = callback;
}

void MIDISequencer::setOnNoteOffCallback(std::function<void(int, int)> callback) {
    onNoteOffCallback_ = callback;
}

void MIDISequencer::setOnEndCallback(std::function<void()> callback) {
    onEndCallback_ = callback;
}

}
}