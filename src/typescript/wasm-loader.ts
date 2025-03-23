import { RhubarbOptions, LipSyncResult } from "./index";

declare const Module: any;

export interface RhubarbWasmModule {
  getLipSync: (
    audioBase64: string,
    options?: RhubarbOptions
  ) => Promise<LipSyncResult>;
}

let wasmModule: RhubarbWasmModule | null = null;

export async function initWasmModule(): Promise<RhubarbWasmModule> {
  if (!wasmModule) {
    // Import the WASM module dynamically
    require("./wasm/rhubarb.js");

    // Wait for the module to be ready
    await new Promise<void>((resolve) => {
      if (Module.ready) {
        resolve();
      } else {
        Module.ready = resolve;
      }
    });

    wasmModule = {
      getLipSync: async (audioBase64: string, options: RhubarbOptions = {}) => {
        // Convert base64 to Float32Array
        const binaryString = Buffer.from(audioBase64, "base64").toString(
          "binary"
        );
        const bytes = new Uint8Array(binaryString.length);
        for (let i = 0; i < binaryString.length; i++) {
          bytes[i] = binaryString.charCodeAt(i);
        }
        const audioData = new Float32Array(bytes.buffer);

        // PocketSphinx requires 16kHz audio
        const result = await Module.getLipSync(
          audioData,
          16000,
          options.dialogText || ""
        );
        return {
          mouthCues: result.map((cue: any) => ({
            start: cue.start,
            end: cue.end,
            value: cue.phoneme,
          })),
        };
      },
    };
  }
  if (!wasmModule) {
    throw new Error("Failed to initialize WASM module");
  }
  return wasmModule;
}

export function getWasmModule(): RhubarbWasmModule | null {
  return wasmModule;
}
