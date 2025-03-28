# Rhubarb Lip Sync WASM

A WebAssembly port of [Rhubarb Lip Sync](https://github.com/DanielSWolf/rhubarb-lip-sync) optimized for web applications. This package provides lip-sync functionality by analyzing audio files and generating mouth shapes for animation. Perfect for creating realistic mouth movements for AI agents, virtual characters, and interactive applications.

## Acknowledgments

This project is a WebAssembly port of [Rhubarb Lip Sync](https://github.com/DanielSWolf/rhubarb-lip-sync), originally created by [Daniel S. Wolf](https://github.com/DanielSWolf). The original project is an amazing piece of software that provides high-quality lip sync from audio files.

All credit for the core lip sync technology and algorithms goes to Daniel S. Wolf and the contributors of the original project. This WebAssembly port aims to make this technology more accessible to web developers while maintaining the high quality of the original implementation.

## Use Cases

This package is particularly useful for:

- AI Agents and Virtual Characters

  - Chatbots with animated avatars
  - Virtual presenters and hosts
  - Digital humans and AI assistants
  - Interactive virtual characters
  - Conversational AI interfaces

- Interactive Applications
  - Educational content with animated characters
  - Virtual meetings and presentations
  - Gaming and entertainment
  - Virtual try-on experiences
  - Interactive storytelling

## Installation

```bash
npm install rhubarb-lip-sync-wasm
# or
yarn add rhubarb-lip-sync-wasm
```

## Usage

```typescript
import { Rhubarb } from "rhubarb-lip-sync-wasm";

// Example usage with an AI agent's audio response
async function generateLipSync(pcmBuffer: Buffer<ArrayBuffer>) {
  try {
    const result = await Rhubarb.getLipSync(pcmBuffer, {
      dialogText: "Optional dialog text for better recognition",
    });

    // result.mouthCues contains an array of mouth shapes with timings
    console.log(result.mouthCues);
  } catch (error) {
    console.error("Error generating lip sync:", error);
  }
}
```

## API

### Rhubarb.getLipSync(audioBase64: string, options?: RhubarbOptions)

Generates lip sync data from an audio file.

#### Parameters

- `pcmBuffer`: Buffer<ArrayBuffer> - Raw PCM audio buffer (16KHz)
- `options`: RhubarbOptions (optional)
  - `dialogText`: string - Optional text to guide the recognition process. If provided, it helps PocketSphinx better recognize the speech. If not provided, PocketSphinx will perform recognition without text guidance.

#### Returns

Promise<LipSyncResult> containing:

- `mouthCues`: Array of mouth shapes with timings
  - `start`: number - Start time in seconds
  - `end`: number - End time in seconds
  - `value`: string - Mouth shape value (A-H, X)

## Development

This package requires Emscripten to be installed for building the WASM module. Make sure you have it installed before running the build commands.

```bash
# Install dependencies
yarn install

# Build the package
yarn build
```

## How It Works

This package uses WebAssembly to port the C++ implementation of Rhubarb Lip Sync to the web. The original Rhubarb Lip Sync uses PocketSphinx for speech recognition and advanced audio processing algorithms.

The WebAssembly port maintains these features while making them available in a JavaScript/TypeScript environment.

## Differences from Original Implementation

While this port aims to maintain feature parity with the original Rhubarb Lip Sync, there are some differences due to the web environment:

1. Input is limited to raw PCM 16KHz audio buffer
2. All processing is done in-memory
3. Optimized for use in web applications
4. Added TypeScript support
5. Simplified API for web use
6. Uses PocketSphinx for speech recognition (requires 16kHz sample rate audio input)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

The original Rhubarb Lip Sync is also licensed under the MIT License. See the [original project](https://github.com/DanielSWolf/rhubarb-lip-sync) for more details.

This project is a WebAssembly port of Rhubarb Lip Sync (https://github.com/DanielSWolf/rhubarb-lip-sync)
Original work Copyright (c) Daniel S. Wolf

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.
