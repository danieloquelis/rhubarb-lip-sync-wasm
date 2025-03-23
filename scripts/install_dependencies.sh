#!/bin/bash

# Install system dependencies
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    brew install cmake pkg-config
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    sudo apt-get update
    sudo apt-get install -y cmake pkg-config
else
    echo "Unsupported operating system"
    exit 1
fi

# Install Emscripten if not already installed
if ! command -v emcc &> /dev/null; then
    echo "Installing Emscripten..."
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    cd ..
fi

echo "Dependencies installed successfully!" 