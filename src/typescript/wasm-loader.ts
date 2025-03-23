import { RhubarbOptions, LipSyncResult } from './index';

declare const Module: any;

export interface RhubarbWasmModule {
  getLipSync: (audioBase64: string, dialogText: string) => Promise<LipSyncResult>;
}

let wasmModule: RhubarbWasmModule | null = null;

export async function initWasmModule(): Promise<RhubarbWasmModule> {
  if (!wasmModule) {
    // Import the WASM module dynamically
    require('../../dist/wasm/rhubarb.js');
    
    // Wait for the module to be ready
    await new Promise<void>((resolve) => {
      if (Module.ready) {
        resolve();
      } else {
        Module.ready = resolve;
      }
    });

    wasmModule = {
      getLipSync: async (audioBase64: string, dialogText: string) => {
        // Convert base64 to Float32Array
        const binaryString = Buffer.from(audioBase64, 'base64').toString('binary');
        const bytes = new Uint8Array(binaryString.length);
        for (let i = 0; i < binaryString.length; i++) {
          bytes[i] = binaryString.charCodeAt(i);
        }
        const audioData = new Float32Array(bytes.buffer);
        
        const result = await Module.getLipSync(audioData, 44100); // Assuming 44.1kHz sample rate
        return {
          mouthCues: result.map((cue: any) => ({
            start: cue.start,
            end: cue.end,
            value: cue.phoneme
          }))
        };
      }
    };
  }
  return wasmModule;
}

export function getWasmModule(): RhubarbWasmModule | null {
  return wasmModule;
} 