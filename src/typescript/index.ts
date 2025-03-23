import { RhubarbOptions, LipSyncResult } from './types.js';
import { initWasmModule, getLipSyncData } from './wasm-loader.js';

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
  private static instance: Rhubarb;
  private wasmModule: any;

  private constructor() {
    this.wasmModule = null;
  }

  public static async getInstance(): Promise<Rhubarb> {
    if (!Rhubarb.instance) {
      Rhubarb.instance = new Rhubarb();
    }
    return Rhubarb.instance;
  }

  async initialize(): Promise<void> {
    if (!this.wasmModule) {
      this.wasmModule = await initWasmModule();
    }
  }

  async getLipSync(audioBase64: string, options: RhubarbOptions = {}): Promise<LipSyncResult> {
    await this.initialize();
    if (!this.wasmModule) {
      throw new Error('WASM module not initialized');
    }
    return getLipSyncData(audioBase64, options.dialogText);
  }
}

export type { RhubarbOptions, LipSyncResult }; 