/// <reference types="node" />

import { fileURLToPath } from "url";
import { dirname, join } from "path";
import { RhubarbWasmModule, LipSyncResult } from "./types.js";

let wasmModule: RhubarbWasmModule | null = null;

export async function initWasmModule(): Promise<RhubarbWasmModule> {
  const __filename = fileURLToPath(import.meta.url);
  const __dirname = dirname(__filename);

  const module = await import("./wasm/rhubarb.js");
  const instance = await module.default({
    locateFile: (path: string) => {
      if (path.endsWith(".wasm") || path.endsWith(".data")) {
        return join(__dirname, "wasm", path);
      }
      return path;
    },
  });

  wasmModule = instance;
  return instance;
}

/**
 * Generate lip sync data from PCM audio data
 * @param pcmData Buffer containing 16-bit PCM audio data at 16kHz mono
 * @param dialogText Optional dialog text for improved recognition
 * @returns Promise resolving to lip sync result with mouth cues
 */
export async function getLipSyncData(
  pcmData: Buffer<ArrayBuffer>,
  dialogText?: string
): Promise<LipSyncResult> {
  if (!Buffer.isBuffer(pcmData)) {
    throw new Error('pcmData must be a Buffer containing 16-bit PCM audio data at 16kHz mono');
  }

  const module = await initWasmModule();
  const result = module.getLipSync(pcmData, dialogText || "");
  return result;
}

export function getWasmModule(): RhubarbWasmModule | null {
  return wasmModule;
}
