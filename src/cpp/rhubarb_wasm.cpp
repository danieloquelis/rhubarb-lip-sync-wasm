#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include "rhubarb/src/recognition/PocketSphinxRecognizer.h"
#include "rhubarb/src/recognition/Recognizer.h"
#include "rhubarb/src/audio/audioFileReading.h"
#include "rhubarb/src/audio/AudioClip.h"
#include "rhubarb/src/audio/BufferAudioClip.h"
#include "rhubarb/src/tools/progress.h"
#include "rhubarb/src/core/Shape.h"

using namespace emscripten;

struct MouthCue {
  double start;
  double end;
  std::string value;
};

struct LipSyncResult {
  std::vector<MouthCue> mouthCues;
};

// Simple progress sink implementation
class WebProgressSink : public ProgressSink {
public:
    void reportProgress(double progress) override {
        // In the future, we could emit a JavaScript event here
        // Uncomment to see progress
        //std::cout << "Progress: " << (progress * 100) << "%" << std::endl;
    }
};

// Base64 decoding table
static const unsigned char base64_decode_table[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0
};

// Convert base64 string to audio buffer
std::vector<float> base64ToAudioBuffer(const std::string& base64) {
    std::vector<float> audioBuffer;
    const char* input = base64.c_str();
    size_t inputLength = base64.length();
    
    // Remove padding if present
    while (inputLength > 0 && input[inputLength - 1] == '=') {
        inputLength--;
    }
    
    // Calculate output length
    size_t outputLength = (inputLength * 3) / 4;
    audioBuffer.resize(outputLength / sizeof(float));
    
    // Decode base64
    size_t j = 0;
    for (size_t i = 0; i < inputLength; i += 4) {
        unsigned char b1 = base64_decode_table[input[i]];
        unsigned char b2 = base64_decode_table[input[i + 1]];
        unsigned char b3 = base64_decode_table[input[i + 2]];
        unsigned char b4 = base64_decode_table[input[i + 3]];
        
        unsigned int value = (b1 << 18) | (b2 << 12) | (b3 << 6) | b4;
        
        // Convert to float
        for (size_t k = 0; k < 3 && j < outputLength; k++) {
            unsigned char byte = (value >> (16 - k * 8)) & 0xFF;
            float sample = (float)byte / 127.5f - 1.0f;
            audioBuffer[j++] = sample;
        }
    }
    
    return audioBuffer;
}

// Convert audio buffer to AudioClip
std::unique_ptr<AudioClip> createAudioClip(const std::vector<float>& buffer) {
    int sampleRate = 16000; // PocketSphinx expects 16kHz audio
    return std::make_unique<BufferAudioClip>(buffer.data(), buffer.size(), sampleRate);
}

// Convert Phone to Shape character
char getShapeChar(const Phone& phone) {
    // This is a simplified mapping. You might want to implement a more sophisticated one.
    switch (phone) {
        case Phone::P:
        case Phone::B:
        case Phone::M:
            return 'A';  // Closed mouth
        case Phone::IY:
        case Phone::T:
        case Phone::D:
        case Phone::N:
        case Phone::S:
        case Phone::Z:
            return 'B';  // Clenched teeth
        case Phone::EH:
        case Phone::AH:
            return 'C';  // Open mouth
        case Phone::AA:
        case Phone::AE:
        case Phone::AY:
            return 'D';  // Mouth wide open
        case Phone::AO:
        case Phone::OW:
            return 'E';  // Rounded mouth
        case Phone::UW:
        case Phone::W:
        case Phone::OY:
            return 'F';  // Puckered lips
        case Phone::F:
        case Phone::V:
            return 'G';  // "F", "V"
        case Phone::L:
            return 'H';  // "L"
        default:
            return 'X';  // Idle
    }
}

// Main function to process audio and generate lip sync data
emscripten::val getLipSync(const std::string& audioBase64, const std::string& dialogText = "") {
    LipSyncResult result;
    
    try {
        // Convert base64 to audio buffer
        std::vector<float> audioBuffer = base64ToAudioBuffer(audioBase64);
        
        // Create audio clip
        auto audioClip = createAudioClip(audioBuffer);
        
        // Create PocketSphinx recognizer
        auto recognizer = std::make_unique<PocketSphinxRecognizer>();
        
        // Create progress sink
        WebProgressSink progressSink;
        
        // Process audio and get recognition result
        boost::optional<std::string> dialog = dialogText.empty() ? boost::none : boost::optional<std::string>(dialogText);
        auto recognitionResult = recognizer->recognizePhones(*audioClip, dialog, 4, progressSink);
        
        // Convert recognition result to mouth cues
        for (const auto& entry : recognitionResult) {
            const auto& timeRange = entry.getTimeRange();
            const auto& phone = entry.getValue();
            
            result.mouthCues.push_back({
                timeRange.getStart().count() / 100.0,  // Convert centiseconds to seconds
                timeRange.getEnd().count() / 100.0,    // Convert centiseconds to seconds
                std::string(1, getShapeChar(phone))    // Convert phone to shape character
            });
        }
        
    } catch (const std::exception& e) {
        // Keep error logging for debugging
        std::cerr << "Error processing audio: " << e.what() << std::endl;
    }
    
    // Convert vector to JavaScript array
    emscripten::val mouthCuesArray = emscripten::val::array();
    for (const auto& cue : result.mouthCues) {
        emscripten::val cueObj = emscripten::val::object();
        cueObj.set("start", cue.start);
        cueObj.set("end", cue.end);
        cueObj.set("value", cue.value);
        mouthCuesArray.call<void>("push", cueObj);
    }
    
    emscripten::val resultObj = emscripten::val::object();
    resultObj.set("mouthCues", mouthCuesArray);
    return resultObj;
}

// Bind C++ functions to JavaScript
EMSCRIPTEN_BINDINGS(rhubarb_wasm) {
    // Register the MouthCue type
    value_object<MouthCue>("MouthCue")
        .field("start", &MouthCue::start)
        .field("end", &MouthCue::end)
        .field("value", &MouthCue::value);
        
    // Register the vector<MouthCue> type
    register_vector<MouthCue>("VectorMouthCue");
        
    // Register the LipSyncResult type
    value_object<LipSyncResult>("LipSyncResult")
        .field("mouthCues", &LipSyncResult::mouthCues);
        
    // Register the getLipSync function with proper return type
    function("getLipSync", &getLipSync, allow_raw_pointers());
} 