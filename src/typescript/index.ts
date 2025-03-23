import { RhubarbOptions, LipSyncResult, RhubarbWasmModule } from './types.js';
import { initWasmModule } from './wasm-loader.js';

declare global {
  interface Window {
    RhubarbWasm: {
      getLipSync: (
        audioBase64: string,
        options?: RhubarbOptions
      ) => Promise<LipSyncResult>;
    };
  }
}

export class Rhubarb {
  private static wasmModule: Promise<RhubarbWasmModule> | null = null;

  private static async getModule(): Promise<RhubarbWasmModule> {
    if (!this.wasmModule) {
      this.wasmModule = initWasmModule();
    }
    return this.wasmModule;
  }

  static async getLipSync(audioBase64: string, options: RhubarbOptions = {}): Promise<LipSyncResult> {
    const module = await this.getModule();
    return module.getLipSync(audioBase64, options.dialogText || '');
  }
}

export type { RhubarbOptions, LipSyncResult }; 