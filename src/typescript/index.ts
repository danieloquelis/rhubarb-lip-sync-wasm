import { RhubarbOptions, LipSyncResult, RhubarbWasmModule } from "./types.js";
import { initWasmModule } from "./wasm-loader.js";

declare global {
  interface Window {
    RhubarbWasm: {
      getLipSync: (
        pcmData: Buffer<ArrayBuffer>,
        options?: RhubarbOptions
      ) => Promise<LipSyncResult>;
    };
  }
}

/**
 * Main Rhubarb class for lip sync generation
 */
export class Rhubarb {
  private static wasmModule: Promise<RhubarbWasmModule> | null = null;

  private static async getModule(): Promise<RhubarbWasmModule> {
    if (!this.wasmModule) {
      this.wasmModule = initWasmModule();
    }
    return this.wasmModule;
  }

  /**
   * Generate lip sync data from PCM audio data
   * @param pcmData Buffer containing 16-bit PCM audio data at 16kHz mono
   * @param options Optional parameters including dialog text
   * @returns Promise resolving to lip sync result with mouth cues
   */
  static async getLipSync(
    pcmData: Buffer<ArrayBuffer>,
    options: RhubarbOptions = {}
  ): Promise<LipSyncResult> {
    if (!Buffer.isBuffer(pcmData)) {
      throw new Error('pcmData must be a Buffer containing 16-bit PCM audio data at 16kHz mono');
    }

    const module = await this.getModule();
    return module.getLipSync(pcmData, options.dialogText || "");
  }
}

export type { RhubarbOptions, LipSyncResult };
