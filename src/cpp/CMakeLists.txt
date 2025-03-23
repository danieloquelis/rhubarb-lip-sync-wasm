cmake_minimum_required(VERSION 3.10)
project(rhubarb_wasm)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Boost include directory
set(BOOST_INCLUDE_DIR "/opt/homebrew/include")

# Build SphinxBase
file(GLOB SPHINXBASE_SOURCES
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/util/*.c"
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/fe/*.c"
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/feat/*.c"
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/lm/*.c"
)
add_library(sphinxbase STATIC ${SPHINXBASE_SOURCES})
target_include_directories(sphinxbase PUBLIC 
    "rhubarb/lib/sphinxbase-rev13216/include"
    "rhubarb/lib/sphinxbase/include"
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/util"
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/fe"
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/feat"
    "rhubarb/lib/sphinxbase-rev13216/src/libsphinxbase/lm"
)

# Create a header file for sphinxbase-specific definitions
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sphinxbase_config.h"
"#ifndef SPHINXBASE_CONFIG_H
#define SPHINXBASE_CONFIG_H

#define HAVE_CONFIG_H
#define FIXED_POINT 1
#define NO_PROFILING 1
#define F2C_LITE
#define BLAS_LITE
#define LAPACK_LITE
#define FLOAT 1
#define DOUBLE 1
#define INTEGER 1
#define LOGICAL 1
#define VOID 1

#endif // SPHINXBASE_CONFIG_H
")

target_include_directories(sphinxbase PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(sphinxbase PUBLIC HAVE_CONFIG_H FIXED_POINT=1 NO_PROFILING=1)

# Build PocketSphinx
file(GLOB POCKETSPHINX_SOURCES
    "rhubarb/lib/pocketsphinx-rev13216/src/libpocketsphinx/*.c"
)
add_library(pocketsphinx STATIC ${POCKETSPHINX_SOURCES})
target_include_directories(pocketsphinx PUBLIC 
    "rhubarb/lib/pocketsphinx-rev13216/include"
    "rhubarb/lib/pocketsphinx-rev13216/src/libpocketsphinx"
    "rhubarb/lib/sphinxbase-rev13216/include"
    "rhubarb/lib/sphinxbase/include"
)
target_compile_definitions(pocketsphinx PUBLIC NO_PROFILING=1)
target_link_libraries(pocketsphinx PUBLIC sphinxbase)

# Add cppformat library
file(GLOB cppFormatFiles "rhubarb/lib/cppformat/*.cc")
add_library(cppFormat ${cppFormatFiles})
target_include_directories(cppFormat PUBLIC "rhubarb/lib/cppformat")

# Build Flite library
set(FLITE_SOURCES
    "rhubarb/lib/flite/src/audio/au_none.c"
    "rhubarb/lib/flite/src/audio/au_streaming.c"
    "rhubarb/lib/flite/src/audio/audio.c"
    "rhubarb/lib/flite/src/hrg/cst_ffeature.c"
    "rhubarb/lib/flite/src/hrg/cst_item.c"
    "rhubarb/lib/flite/src/hrg/cst_relation.c"
    "rhubarb/lib/flite/src/hrg/cst_utterance.c"
    "rhubarb/lib/flite/src/lexicon/cst_lexicon.c"
    "rhubarb/lib/flite/src/lexicon/cst_lts.c"
    "rhubarb/lib/flite/src/regex/cst_regex.c"
    "rhubarb/lib/flite/src/regex/regexp.c"
    "rhubarb/lib/flite/src/speech/cst_lpcres.c"
    "rhubarb/lib/flite/src/speech/cst_track.c"
    "rhubarb/lib/flite/src/speech/cst_wave.c"
    "rhubarb/lib/flite/src/speech/cst_wave_io.c"
    "rhubarb/lib/flite/src/speech/cst_wave_utils.c"
    "rhubarb/lib/flite/src/speech/rateconv.c"
    "rhubarb/lib/flite/src/stats/cst_cart.c"
    "rhubarb/lib/flite/src/synth/cst_ffeatures.c"
    "rhubarb/lib/flite/src/synth/cst_phoneset.c"
    "rhubarb/lib/flite/src/synth/cst_synth.c"
    "rhubarb/lib/flite/src/synth/cst_utt_utils.c"
    "rhubarb/lib/flite/src/synth/cst_voice.c"
    "rhubarb/lib/flite/src/synth/flite.c"
    "rhubarb/lib/flite/src/utils/cst_alloc.c"
    "rhubarb/lib/flite/src/utils/cst_endian.c"
    "rhubarb/lib/flite/src/utils/cst_error.c"
    "rhubarb/lib/flite/src/utils/cst_features.c"
    "rhubarb/lib/flite/src/utils/cst_file_stdio.c"
    "rhubarb/lib/flite/src/utils/cst_string.c"
    "rhubarb/lib/flite/src/utils/cst_tokenstream.c"
    "rhubarb/lib/flite/src/utils/cst_val.c"
    "rhubarb/lib/flite/src/utils/cst_val_const.c"
    "rhubarb/lib/flite/src/utils/cst_val_user.c"
    "rhubarb/lib/flite/lang/usenglish/us_aswd.c"
    "rhubarb/lib/flite/lang/usenglish/us_dur_stats.c"
    "rhubarb/lib/flite/lang/usenglish/us_durz_cart.c"
    "rhubarb/lib/flite/lang/usenglish/us_expand.c"
    "rhubarb/lib/flite/lang/usenglish/us_f0_model.c"
    "rhubarb/lib/flite/lang/usenglish/us_f0lr.c"
    "rhubarb/lib/flite/lang/usenglish/us_ffeatures.c"
    "rhubarb/lib/flite/lang/usenglish/us_gpos.c"
    "rhubarb/lib/flite/lang/usenglish/us_int_accent_cart.c"
    "rhubarb/lib/flite/lang/usenglish/us_int_tone_cart.c"
    "rhubarb/lib/flite/lang/usenglish/us_nums_cart.c"
    "rhubarb/lib/flite/lang/usenglish/us_phoneset.c"
    "rhubarb/lib/flite/lang/usenglish/us_phrasing_cart.c"
    "rhubarb/lib/flite/lang/usenglish/us_pos_cart.c"
    "rhubarb/lib/flite/lang/usenglish/us_text.c"
    "rhubarb/lib/flite/lang/usenglish/usenglish.c"
    "rhubarb/lib/flite/lang/cmulex/cmu_lex.c"
    "rhubarb/lib/flite/lang/cmulex/cmu_lex_data.c"
    "rhubarb/lib/flite/lang/cmulex/cmu_lex_entries.c"
    "rhubarb/lib/flite/lang/cmulex/cmu_lts_model.c"
    "rhubarb/lib/flite/lang/cmulex/cmu_lts_rules.c"
    "rhubarb/lib/flite/lang/cmulex/cmu_postlex.c"
)
add_library(flite STATIC ${FLITE_SOURCES})
target_include_directories(flite PUBLIC 
    "rhubarb/lib/flite/include"
    "rhubarb/lib/flite/include/flite"
    "rhubarb/lib/flite/lang/usenglish"
    "rhubarb/lib/flite/lang/cmulex"
)

# Build WebRTC VAD
file(GLOB WEBRTC_SOURCES
    "rhubarb/lib/webrtc/webrtc/common_audio/signal_processing/*.c"
    "rhubarb/lib/webrtc/webrtc/common_audio/vad/*.c"
)
# Exclude MIPS-specific files
list(FILTER WEBRTC_SOURCES EXCLUDE REGEX ".*_mips\\.c$")
# Exclude NEON-specific files
list(FILTER WEBRTC_SOURCES EXCLUDE REGEX ".*_neon\\.c$")
add_library(webRtc STATIC ${WEBRTC_SOURCES})
target_include_directories(webRtc PUBLIC 
    "rhubarb/lib/webrtc"
    "rhubarb/lib/webrtc/webrtc"
    "rhubarb/lib/webrtc/webrtc/common_audio"
    "rhubarb/lib/webrtc/webrtc/common_audio/signal_processing"
    "rhubarb/lib/webrtc/webrtc/common_audio/vad"
)
target_compile_definitions(webRtc PUBLIC WEBRTC_POSIX)

# Build utf8proc
add_library(utf8proc STATIC
    "rhubarb/lib/utf8proc/utf8proc.c"
)
target_include_directories(utf8proc PUBLIC 
    "rhubarb/lib/utf8proc"
)
target_compile_definitions(utf8proc PUBLIC UTF8PROC_STATIC=1)

# Build whereami
add_library(whereami STATIC
    "rhubarb/lib/whereami/src/whereami.c"
)
target_include_directories(whereami PUBLIC 
    "rhubarb/lib/whereami/src"
)

# Build utfcpp (header-only)
add_library(utfcpp INTERFACE)
target_include_directories(utfcpp INTERFACE 
    "rhubarb/lib/utf8cpp"
)

# Add app info
add_definitions(-DAPP_NAME="rhubarb-wasm")
add_definitions(-DAPP_VERSION="1.0.0")

# Define source files
set(SOURCES
    rhubarb_wasm.cpp
    appInfo.cpp
    # Core files
    rhubarb/src/core/Phone.cpp
    rhubarb/src/core/Shape.cpp
    # Recognition files
    rhubarb/src/recognition/PocketSphinxRecognizer.cpp
    rhubarb/src/recognition/pocketSphinxTools.cpp
    rhubarb/src/recognition/languageModels.cpp
    rhubarb/src/recognition/tokenization.cpp
    rhubarb/src/recognition/g2p.cpp
    # Audio files
    rhubarb/src/audio/AudioClip.cpp
    rhubarb/src/audio/AudioSegment.cpp
    rhubarb/src/audio/audioFileReading.cpp
    rhubarb/src/audio/processing.cpp
    rhubarb/src/audio/DcOffset.cpp
    rhubarb/src/audio/voiceActivityDetection.cpp
    rhubarb/src/audio/SampleRateConverter.cpp
    # Time files
    rhubarb/src/time/TimeRange.cpp
    rhubarb/src/time/centiseconds.cpp
    # Tools files
    rhubarb/src/tools/progress.cpp
    rhubarb/src/tools/ProgressBar.cpp
    rhubarb/src/tools/StringTools.cpp
    rhubarb/src/tools/TablePrinter.cpp
    rhubarb/src/tools/Tools.cpp
    rhubarb/src/tools/exceptions.cpp
    rhubarb/src/tools/platformTools.cpp
    rhubarb/src/tools/textFiles.cpp
    # Logging files
    rhubarb/src/logging/formatters.cpp
    rhubarb/src/logging/logging.cpp
    rhubarb/src/logging/sinks.cpp
    rhubarb/src/logging/Entry.cpp
    rhubarb/src/logging/Level.cpp
)

# Create WASM library
add_executable(rhubarb_wasm ${SOURCES})
set_target_properties(rhubarb_wasm PROPERTIES
    OUTPUT_NAME "rhubarb"
    LINK_FLAGS "-s WASM=1 -s EXPORT_ES6=1 -s SINGLE_FILE=0 -s EXPORTED_FUNCTIONS='[\"_malloc\", \"_free\"]' -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\", \"getValue\", \"setValue\"]' -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=2 -s LINKABLE=1 -s EXPORT_ALL=1 -s NO_DISABLE_EXCEPTION_CATCHING=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/rhubarb/res@/res"
)

target_compile_options(rhubarb_wasm PRIVATE "-fexceptions")
target_link_options(rhubarb_wasm PRIVATE "-lembind")

# Define NO_PROFILING before any SphinxBase headers
target_compile_definitions(rhubarb_wasm PRIVATE 
    NO_PROFILING=1
    HAVE_CONFIG_H
    FIXED_POINT=1
)

# Include directories
target_include_directories(rhubarb_wasm BEFORE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "rhubarb/lib/pocketsphinx-rev13216/include"
    "rhubarb/lib/pocketsphinx-rev13216/src/libpocketsphinx"
    "rhubarb/lib/sphinxbase-rev13216/include"
    "rhubarb/lib/sphinxbase/include"
    "rhubarb/lib/flite/include"
    "rhubarb/lib/flite/include/flite"
    "rhubarb/lib/flite/lang/usenglish"
    "rhubarb/lib/flite/lang/cmulex"
    "rhubarb/src"
    "rhubarb/src/core"
    "rhubarb/src/recognition"
    "rhubarb/src/audio"
    "rhubarb/src/time"
    "rhubarb/src/tools"
    "rhubarb/src/logging"
    ${BOOST_INCLUDE_DIR}
    "rhubarb/lib/utf8proc"
    "rhubarb/lib/gsl/include"
    "rhubarb/lib/webrtc"
    "rhubarb/lib/cppformat"
)

# Add pre-include header
target_compile_options(rhubarb_wasm PRIVATE "-include${CMAKE_CURRENT_SOURCE_DIR}/preinclude.h")

# Link libraries
target_link_libraries(rhubarb_wasm 
    cppFormat
    pocketsphinx
    sphinxbase
    flite
    webRtc
    utf8proc
    whereami
    utfcpp
) 