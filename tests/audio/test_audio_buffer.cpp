#include <gtest/gtest.h>
#include "../../src/audio/audio_buffer.h"
#include "../utils/test_data_generator.h"
#include "../utils/test_utils.h"
#include <vector>

using namespace v3d::audio;
using namespace v3d::test;

class AudioBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        AudioSpec spec;
        spec.sampleRate = 44100;
        spec.format = AudioFormat::Float32;
        spec.channels = AudioChannelLayout::Stereo;
        
        buffer_ = std::make_shared<AudioBuffer>(spec, 44100);
    }

    void TearDown() override {
    }

    std::shared_ptr<AudioBuffer> buffer_;
};

TEST_F(AudioBufferTest, GetSpec) {
    const AudioSpec& spec = buffer_->getSpec();
    
    EXPECT_EQ(spec.sampleRate, 44100);
    EXPECT_EQ(spec.format, AudioFormat::Float32);
    EXPECT_EQ(spec.channels, AudioChannelLayout::Stereo);
}

TEST_F(AudioBufferTest, GetFrameCount) {
    size_t frameCount = buffer_->getFrameCount();
    
    EXPECT_EQ(frameCount, 44100);
}

TEST_F(AudioBufferTest, GetSampleCount) {
    size_t sampleCount = buffer_->getSampleCount();
    
    EXPECT_EQ(sampleCount, 44100 * 2);
}

TEST_F(AudioBufferTest, GetSizeInBytes) {
    size_t sizeInBytes = buffer_->getSizeInBytes();
    
    EXPECT_EQ(sizeInBytes, 44100 * 2 * sizeof(float));
}

TEST_F(AudioBufferTest, GetData) {
    void* data = buffer_->getData();
    const void* constData = buffer_->getData();
    
    EXPECT_NE(data, nullptr);
    EXPECT_NE(constData, nullptr);
}

TEST_F(AudioBufferTest, Clear) {
    float* data = static_cast<float*>(buffer_->getData());
    data[0] = 1.0f;
    data[1] = 2.0f;
    
    buffer_->clear();
    
    EXPECT_FLOAT_EQ(data[0], 0.0f);
    EXPECT_FLOAT_EQ(data[1], 0.0f);
}

TEST_F(AudioBufferTest, Fill) {
    buffer_->fill(0.5f);
    
    float* data = static_cast<float*>(buffer_->getData());
    
    EXPECT_FLOAT_EQ(data[0], 0.5f);
    EXPECT_FLOAT_EQ(data[1], 0.5f);
}

TEST_F(AudioBufferTest, ApplyGain) {
    float* data = static_cast<float*>(buffer_->getData());
    data[0] = 1.0f;
    data[1] = 2.0f;
    
    buffer_->applyGain(0.5f);
    
    EXPECT_FLOAT_EQ(data[0], 0.5f);
    EXPECT_FLOAT_EQ(data[1], 1.0f);
}

TEST_F(AudioBufferTest, ApplyFadeIn) {
    buffer_->fill(1.0f);
    buffer_->applyFadeIn(1000);
    
    float* data = static_cast<float*>(buffer_->getData());
    
    EXPECT_FLOAT_EQ(data[0], 0.0f);
    EXPECT_FLOAT_EQ(data[999], 1.0f);
}

TEST_F(AudioBufferTest, ApplyFadeOut) {
    buffer_->fill(1.0f);
    buffer_->applyFadeOut(1000);
    
    float* data = static_cast<float*>(buffer_->getData());
    
    EXPECT_FLOAT_EQ(data[0], 1.0f);
    EXPECT_FLOAT_EQ(data[999], 0.0f);
}

TEST_F(AudioBufferTest, CopyFrom) {
    float* srcData = static_cast<float*>(buffer_->getData());
    srcData[0] = 1.0f;
    srcData[1] = 2.0f;
    
    AudioSpec spec;
    spec.sampleRate = 44100;
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Stereo;
    
    AudioBuffer destBuffer(spec, 44100);
    destBuffer.copyFrom(*buffer_);
    
    float* destData = static_cast<float*>(destBuffer.getData());
    
    EXPECT_FLOAT_EQ(destData[0], 1.0f);
    EXPECT_FLOAT_EQ(destData[1], 2.0f);
}

TEST_F(AudioBufferTest, MixFrom) {
    float* data1 = static_cast<float*>(buffer_->getData());
    data1[0] = 0.5f;
    data1[1] = 0.5f;
    
    AudioSpec spec;
    spec.sampleRate = 44100;
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Stereo;
    
    AudioBuffer buffer2(spec, 44100);
    float* data2 = static_cast<float*>(buffer2.getData());
    data2[0] = 0.3f;
    data2[1] = 0.3f;
    
    buffer_->mixFrom(buffer2, 1.0f);
    
    EXPECT_FLOAT_EQ(data1[0], 0.8f);
    EXPECT_FLOAT_EQ(data1[1], 0.8f);
}

TEST_F(AudioBufferTest, Clone) {
    float* data = static_cast<float*>(buffer_->getData());
    data[0] = 1.0f;
    data[1] = 2.0f;
    
    AudioBuffer cloned = buffer_->clone();
    
    EXPECT_EQ(cloned.getFrameCount(), buffer_->getFrameCount());
    EXPECT_EQ(cloned.getSpec().sampleRate, buffer_->getSpec().sampleRate);
    
    float* clonedData = static_cast<float*>(cloned.getData());
    EXPECT_FLOAT_EQ(clonedData[0], 1.0f);
    EXPECT_FLOAT_EQ(clonedData[1], 2.0f);
}

TEST_F(AudioBufferTest, AudioSpec_GetBytesPerSample) {
    AudioSpec spec;
    
    spec.format = AudioFormat::PCM8;
    EXPECT_EQ(spec.getBytesPerSample(), 1);
    
    spec.format = AudioFormat::PCM16;
    EXPECT_EQ(spec.getBytesPerSample(), 2);
    
    spec.format = AudioFormat::Float32;
    EXPECT_EQ(spec.getBytesPerSample(), 4);
}

TEST_F(AudioBufferTest, AudioSpec_GetChannelCount) {
    AudioSpec spec;
    
    spec.channels = AudioChannelLayout::Mono;
    EXPECT_EQ(spec.getChannelCount(), 1);
    
    spec.channels = AudioChannelLayout::Stereo;
    EXPECT_EQ(spec.getChannelCount(), 2);
    
    spec.channels = AudioChannelLayout::Surround51;
    EXPECT_EQ(spec.getChannelCount(), 6);
}

TEST_F(AudioBufferTest, SineWaveBuffer) {
    auto buffer = TestDataGenerator::createSineWaveBuffer(440.0f, 44100, 0.1f);
    
    ASSERT_NE(buffer, nullptr);
    
    float* data = static_cast<float*>(buffer->getData());
    
    EXPECT_FLOAT_EQ(data[0], 0.0f);
    EXPECT_GT(std::abs(data[100]), 0.0f);
}

TEST_F(AudioBufferTest, NoiseBuffer) {
    auto buffer = TestDataGenerator::createNoiseBuffer(44100, 0.1f);
    
    ASSERT_NE(buffer, nullptr);
    
    float* data = static_cast<float*>(buffer->getData());
    
    bool hasNonZero = false;
    for (int i = 0; i < 100; ++i) {
        if (std::abs(data[i]) > 0.0f) {
            hasNonZero = true;
            break;
        }
    }
    
    EXPECT_TRUE(hasNonZero);
}

TEST_F(AudioBufferTest, LargeBuffer) {
    const int sampleRate = 48000;
    const float duration = 60.0f;
    
    TestUtils::measureTime("Create large audio buffer", [sampleRate, duration]() {
        AudioSpec spec;
        spec.sampleRate = sampleRate;
        spec.format = AudioFormat::Float32;
        spec.channels = AudioChannelLayout::Stereo;
        
        size_t frameCount = static_cast<size_t>(sampleRate * duration);
        AudioBuffer buffer(spec, frameCount);
        
        EXPECT_EQ(buffer.getFrameCount(), frameCount);
    });
}

TEST_F(AudioBufferTest, StressTest_ManyOperations) {
    auto buffer = TestDataGenerator::createSineWaveBuffer(440.0f, 44100, 1.0f);
    
    TestUtils::measureTime("Many audio operations", [buffer]() {
        for (int i = 0; i < 1000; ++i) {
            buffer->applyGain(0.5f);
            buffer->applyGain(2.0f);
        }
    });
}

TEST_F(AudioBufferTest, StressTest_Mixing) {
    AudioSpec spec;
    spec.sampleRate = 44100;
    spec.format = AudioFormat::Float32;
    spec.channels = AudioChannelLayout::Stereo;
    
    AudioBuffer buffer1(spec, 44100);
    buffer1.fill(0.5f);
    
    TestUtils::measureTime("Many mixing operations", [&buffer1, spec]() {
        for (int i = 0; i < 100; ++i) {
            AudioBuffer buffer2(spec, 44100);
            buffer2.fill(0.1f);
            buffer1.mixFrom(buffer2, 1.0f);
        }
    });
}
