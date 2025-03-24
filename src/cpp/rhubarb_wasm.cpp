#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <map>
#include <array>
#include <chrono>
#include <sstream>
#include <thread>
#include <iostream>
#include <iomanip>
#include <optional>
#include "rhubarb/src/recognition/PocketSphinxRecognizer.h"
#include "rhubarb/src/recognition/Recognizer.h"
#include "rhubarb/src/audio/audioFileReading.h"
#include "rhubarb/src/audio/AudioClip.h"
#include "rhubarb/src/audio/BufferAudioClip.h"
#include "rhubarb/src/tools/progress.h"
#include "rhubarb/src/core/Shape.h"
#include <boost/optional.hpp>

using namespace emscripten;

// Debug logging helper
void debugLog(const std::string& message) {
    std::cerr << "[DEBUG] " << message << std::endl;
}

template<typename T>
std::string formatNumber(T value, int precision = 2) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

struct MouthCue {
  double start;
  double end;
  std::string value;
};

struct LipSyncResult {
  std::vector<MouthCue> mouthCues;
};

// Audio format information
struct AudioFormatInfo {
    int channelCount;
    int frameRate;
    int bitsPerSample;
    
    AudioFormatInfo() : 
        channelCount(1),    // Mono
        frameRate(16000),   // 16kHz
        bitsPerSample(16)   // 16-bit PCM
    {}
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

// Helper function to get shape set for a phone with timing context
ShapeSet getPhoneShapeSet(const Phone& phone, centiseconds duration, centiseconds previousDuration) {
    static const ShapeSet any { Shape::A, Shape::B, Shape::C, Shape::D, Shape::E, Shape::F, Shape::G, Shape::H, Shape::X };
    static const ShapeSet anyOpen { Shape::B, Shape::C, Shape::D, Shape::E, Shape::F, Shape::G, Shape::H };

    switch (phone) {
        case Phone::AO: return { Shape::E };
        case Phone::AA: return { Shape::D };
        case Phone::IY: return { Shape::B };
        case Phone::UW: return { Shape::F };
        case Phone::EH: return { Shape::C };
        case Phone::IH: return { Shape::B };
        case Phone::UH: return { Shape::F };
        case Phone::AH: return duration < 20_cs ? ShapeSet{ Shape::C } : ShapeSet{ Shape::D };
        case Phone::Schwa: return { Shape::B, Shape::C };
        case Phone::AE: return { Shape::C };
        case Phone::EY: return duration < 20_cs ? ShapeSet{ Shape::C, Shape::B } : ShapeSet{ Shape::D, Shape::B };
        case Phone::AY: return duration < 20_cs ? ShapeSet{ Shape::C, Shape::B } : ShapeSet{ Shape::D, Shape::B };
        case Phone::OW: return { Shape::E, Shape::F };
        case Phone::AW: return duration < 30_cs ? ShapeSet{ Shape::C, Shape::E } : ShapeSet{ Shape::D, Shape::E };
        case Phone::OY: return { Shape::E, Shape::B };
        case Phone::ER: return duration < 7_cs ? ShapeSet{ Shape::B, Shape::C } : ShapeSet{ Shape::E };
        
        // Plosives
        case Phone::P:
        case Phone::B: return any;  // Note: Plosive timing handled separately
        case Phone::T:
        case Phone::D: return anyOpen;  // Note: Plosive timing handled separately
        case Phone::K:
        case Phone::G: return { Shape::B, Shape::C, Shape::E, Shape::F, Shape::H };
        
        // Affricates
        case Phone::CH:
        case Phone::JH: return { Shape::B, Shape::F };
        
        // Fricatives
        case Phone::F:
        case Phone::V: return { Shape::G };
        case Phone::TH:
        case Phone::DH:
        case Phone::S:
        case Phone::Z:
        case Phone::SH:
        case Phone::ZH: return { Shape::B, Shape::F };
        case Phone::HH: return any;  // think "m-hm"
        
        // Nasals
        case Phone::M: return { Shape::A };
        case Phone::N: return { Shape::B, Shape::C, Shape::F, Shape::H };
        case Phone::NG: return { Shape::B, Shape::C, Shape::E, Shape::F };
        
        // Liquids and Glides
        case Phone::L: return duration < 20_cs 
            ? ShapeSet{ Shape::B, Shape::E, Shape::F, Shape::H }
            : ShapeSet{ Shape::H };
        case Phone::R: return { Shape::B, Shape::E, Shape::F };
        case Phone::Y: return { Shape::B, Shape::C, Shape::F };
        case Phone::W: return { Shape::F };
        
        // Non-speech sounds
        case Phone::Breath:
        case Phone::Cough:
        case Phone::Smack: return { Shape::C };
        case Phone::Noise: return { Shape::B };
        
        default: return { Shape::X };
    }
}

// Effort matrix for shape transitions
Shape getClosestShape(Shape reference, const ShapeSet& shapes) {
    if (shapes.empty()) {
        return Shape::X;
    }

    static const std::array<std::array<Shape, 9>, 9> effortMatrix = {{
        /* A */ {{ Shape::A, Shape::X, Shape::G, Shape::B, Shape::C, Shape::H, Shape::E, Shape::D, Shape::F }},
        /* B */ {{ Shape::B, Shape::G, Shape::A, Shape::X, Shape::C, Shape::H, Shape::E, Shape::D, Shape::F }},
        /* C */ {{ Shape::C, Shape::H, Shape::B, Shape::G, Shape::D, Shape::A, Shape::X, Shape::E, Shape::F }},
        /* D */ {{ Shape::D, Shape::C, Shape::H, Shape::B, Shape::G, Shape::A, Shape::X, Shape::E, Shape::F }},
        /* E */ {{ Shape::E, Shape::C, Shape::H, Shape::B, Shape::G, Shape::A, Shape::X, Shape::D, Shape::F }},
        /* F */ {{ Shape::F, Shape::B, Shape::G, Shape::A, Shape::X, Shape::C, Shape::H, Shape::E, Shape::D }},
        /* G */ {{ Shape::G, Shape::A, Shape::B, Shape::C, Shape::H, Shape::X, Shape::E, Shape::D, Shape::F }},
        /* H */ {{ Shape::H, Shape::C, Shape::B, Shape::G, Shape::D, Shape::A, Shape::X, Shape::E, Shape::F }},
        /* X */ {{ Shape::X, Shape::A, Shape::G, Shape::B, Shape::C, Shape::H, Shape::E, Shape::D, Shape::F }}
    }};

    const auto& closestShapes = effortMatrix[static_cast<size_t>(reference)];
    for (Shape closestShape : closestShapes) {
        if (shapes.find(closestShape) != shapes.end()) {
            return closestShape;
        }
    }

    return *shapes.begin();  // Fallback to first available shape
}

// Helper function to convert Shape to string
std::string shapeToString(Shape shape) {
    std::ostringstream stream;
    stream << shape;  // This uses the operator<< defined for Shape
    return stream.str();
}

// Process phones and generate mouth cues
std::vector<MouthCue> processPhones(const BoundedTimeline<Phone>& phones, double audioDurationSeconds) {
    std::vector<MouthCue> mouthCues;
    Shape currentShape = Shape::X;
    
    // Add initial X shape if there's a gap at the start
    if (phones.begin() != phones.end() && phones.begin()->getTimeRange().getStart() > 0_cs) {
        mouthCues.push_back({
            0.0,
            phones.begin()->getTimeRange().getStart().count() / 100.0,
            shapeToString(Shape::X)
        });
    }
    
    // Process each phone
    TimeRange lastTimeRange;
    for (auto it = phones.begin(); it != phones.end(); ++it) {
        const Phone& phone = it->getValue();
        const TimeRange& timeRange = it->getTimeRange();
        const centiseconds duration = timeRange.getDuration();
        const centiseconds previousDuration = it != phones.begin() 
            ? std::prev(it)->getTimeRange().getDuration() 
            : 0_cs;
            
        // Add X shape for gaps between phones (silence)
        if (!lastTimeRange.empty() && timeRange.getStart() > lastTimeRange.getEnd()) {
            const double gapStart = lastTimeRange.getEnd().count() / 100.0;
            const double gapEnd = timeRange.getStart().count() / 100.0;
            if (gapEnd - gapStart >= 0.1) { // Only add X for gaps >= 100ms
                mouthCues.push_back({
                    gapStart,
                    gapEnd,
                    shapeToString(Shape::X)
                });
            }
        }
        
        // Get the set of possible shapes for this phone
        ShapeSet shapeSet = getPhoneShapeSet(phone, duration, previousDuration);
        
        // Choose the best shape based on the current shape
        Shape nextShape = getClosestShape(currentShape, shapeSet);
        
        // Special handling for plosives
        if (phone == Phone::P || phone == Phone::B || phone == Phone::T || phone == Phone::D) {
            const centiseconds occlusionDuration = std::min(std::max(previousDuration / 2, 4_cs), 12_cs);
            const centiseconds occlusionStart = timeRange.getStart() - occlusionDuration;
            
            // Add pre-occlusion shape
            if (phone == Phone::P || phone == Phone::B) {
                mouthCues.push_back({
                    occlusionStart.count() / 100.0,
                    timeRange.getStart().count() / 100.0,
                    shapeToString(Shape::A)
                });
            } else {
                mouthCues.push_back({
                    occlusionStart.count() / 100.0,
                    timeRange.getStart().count() / 100.0,
                    shapeToString(Shape::B)
                });
            }
        }
        
        // Add the main shape
        mouthCues.push_back({
            timeRange.getStart().count() / 100.0,
            timeRange.getEnd().count() / 100.0,
            shapeToString(nextShape)
        });
        
        currentShape = nextShape;
        lastTimeRange = timeRange;
    }
    
    // Add final X shape if there's silence at the end
    if (!phones.empty()) {
        const TimeRange& lastPhone = phones.rbegin()->getTimeRange();
        if (lastPhone.getEnd().count() / 100.0 < audioDurationSeconds) {
            mouthCues.push_back({
                lastPhone.getEnd().count() / 100.0,
                audioDurationSeconds,
                shapeToString(Shape::X)
            });
        }
    }
    
    // Consolidate similar consecutive shapes
    std::vector<MouthCue> consolidatedCues;
    for (size_t i = 0; i < mouthCues.size(); ++i) {
        if (consolidatedCues.empty() || 
            consolidatedCues.back().value != mouthCues[i].value ||
            std::abs(consolidatedCues.back().end - mouthCues[i].start) > 0.001) {
            consolidatedCues.push_back(mouthCues[i]);
        } else {
            consolidatedCues.back().end = mouthCues[i].end;
        }
    }
    
    return consolidatedCues;
}

// Convert raw PCM data to float audio buffer
std::vector<float> pcmToAudioBuffer(const emscripten::val& buffer) {
    debugLog("Converting PCM data to audio buffer");
    
    // Get the underlying ArrayBuffer
    emscripten::val arrayBuffer = buffer["buffer"];
    size_t byteLength = buffer["length"].as<size_t>();
    size_t sampleCount = byteLength / sizeof(int16_t);
    
    debugLog("Input byte length: " + std::to_string(byteLength));
    debugLog("Input sample count: " + std::to_string(sampleCount));
    
    // Create a typed array view of the buffer
    emscripten::val int16Array = emscripten::val::global("Int16Array").new_(arrayBuffer);
    
    std::vector<float> audioBuffer(sampleCount);
    float minSample = 0.0f, maxSample = 0.0f;
    
    // Convert 16-bit PCM to normalized float (-1.0 to 1.0)
    for (size_t i = 0; i < sampleCount; i++) {
        int16_t sample = int16Array[i].as<int16_t>();
        float normalizedSample = static_cast<float>(sample) / 32768.0f;
        audioBuffer[i] = normalizedSample;
        
        minSample = std::min(minSample, normalizedSample);
        maxSample = std::max(maxSample, normalizedSample);
    }
    
    debugLog("Audio buffer conversion complete");
    debugLog("Sample range: " + formatNumber(minSample) + " to " + formatNumber(maxSample));
    
    return audioBuffer;
}

// Create AudioClip from processed buffer
std::unique_ptr<AudioClip> createAudioClip(const std::vector<float>& buffer, const AudioFormatInfo& formatInfo) {
    return std::make_unique<BufferAudioClip>(buffer.data(), buffer.size(), formatInfo.frameRate);
}

// Main function to process audio and generate lip sync data
// Note: pcmData is expected to be a Buffer containing 16-bit PCM mono at 16kHz
emscripten::val getLipSync(emscripten::val pcmData, const std::string& dialogText = "") {
    debugLog("Starting lip sync processing");
    LipSyncResult result;
    
    try {
        // Initialize audio format info (we expect 16kHz mono PCM)
        AudioFormatInfo formatInfo;
        debugLog("Audio format - Channels: " + std::to_string(formatInfo.channelCount) + 
                ", Rate: " + std::to_string(formatInfo.frameRate) + 
                ", Bits: " + std::to_string(formatInfo.bitsPerSample));
        
        // Convert PCM to float audio buffer
        std::vector<float> audioBuffer = pcmToAudioBuffer(pcmData);
        debugLog("Audio buffer size: " + std::to_string(audioBuffer.size()));
        
        // Calculate audio duration in seconds
        double audioDurationSeconds = static_cast<double>(audioBuffer.size()) / formatInfo.frameRate;
        debugLog("Audio duration: " + formatNumber(audioDurationSeconds) + "s");
        
        // Create audio clip
        auto audioClip = createAudioClip(audioBuffer, formatInfo);
        debugLog("Audio clip created");
        
        // Create PocketSphinx recognizer
        auto recognizer = std::make_unique<PocketSphinxRecognizer>();
        debugLog("PocketSphinx recognizer created");
        
        // Create progress sink
        WebProgressSink progressSink;
        
        // Process audio and get recognition result with optimal thread count
        boost::optional<std::string> dialog = dialogText.empty() ? boost::none : boost::optional<std::string>(dialogText);
        const int maxThreadCount = std::thread::hardware_concurrency();
        debugLog("Using " + std::to_string(maxThreadCount) + " threads for recognition");
        
        auto recognitionResult = recognizer->recognizePhones(*audioClip, dialog, maxThreadCount, progressSink);
        debugLog("Phone recognition complete");
        
        // Process phones to get mouth cues
        result.mouthCues = processPhones(recognitionResult, audioDurationSeconds);
        debugLog("Generated " + std::to_string(result.mouthCues.size()) + " mouth cues");
        
        // Log the first few mouth cues for debugging
        const size_t maxCuesToLog = 5;
        for (size_t i = 0; i < std::min(result.mouthCues.size(), maxCuesToLog); i++) {
            const auto& cue = result.mouthCues[i];
            debugLog("Cue " + std::to_string(i) + ": " + 
                    formatNumber(cue.start) + "s to " + 
                    formatNumber(cue.end) + "s = " + cue.value);
        }
        
    } catch (const std::exception& e) {
        debugLog("Error processing audio: " + std::string(e.what()));
        throw;
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
        
    // Register the getLipSync function
    function("getLipSync", &getLipSync);
} 