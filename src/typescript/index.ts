export interface RhubarbOptions {
  dialogText?: string;
}

export interface MouthCue {
  start: number;
  end: number;
  value: string;
}

export interface LipSyncResult {
  mouthCues: MouthCue[];
}

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

  private constructor() {}

  public static async getInstance(): Promise<Rhubarb> {
    if (!Rhubarb.instance) {
      Rhubarb.instance = new Rhubarb();
      const { initWasmModule } = await import('./wasm-loader');
      Rhubarb.instance.wasmModule = await initWasmModule();
    }
    return Rhubarb.instance;
  }

  public static async getLipSync(
    audioBase64: string,
    options: RhubarbOptions = {}
  ): Promise<LipSyncResult> {
    const instance = await Rhubarb.getInstance();
    return instance.wasmModule.getLipSync(audioBase64, options);
  }
} 