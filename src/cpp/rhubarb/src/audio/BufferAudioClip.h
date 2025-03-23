#pragma once

#include "AudioClip.h"
#include <vector>
#include <memory>

class BufferAudioClip : public AudioClip {
public:
    BufferAudioClip(const float* data, size_t size, int sampleRate) :
        buffer(data, data + size),
        sampleRate_(sampleRate) {}

    std::unique_ptr<AudioClip> clone() const override {
        return std::make_unique<BufferAudioClip>(buffer.data(), buffer.size(), sampleRate_);
    }

    int getSampleRate() const override {
        return sampleRate_;
    }

    size_type size() const override {
        return buffer.size();
    }

private:
    SampleReader createUnsafeSampleReader() const override {
        return [data = buffer.data()](size_type pos) -> float {
            return data[pos];
        };
    }

    std::vector<float> buffer;
    int sampleRate_;
}; 