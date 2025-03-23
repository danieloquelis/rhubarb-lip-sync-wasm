cmake_minimum_required(VERSION 3.24)

project(rhubarb-wasm)

# Set Emscripten specific flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -s EXPORTED_FUNCTIONS=['_malloc','_free']")

# Add the src/cpp subdirectory
add_subdirectory(src/cpp)

# Set output name
set_target_properties(rhubarb_wasm PROPERTIES
  OUTPUT_NAME "rhubarb"
) 