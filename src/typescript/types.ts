/// <reference types="node" />

export interface RhubarbOptions {
  dialogText?: string;
}

export interface MouthCue {
  start: number;  // Start time in seconds
  end: number;    // End time in seconds
  value: string;  // Shape value (A, B, C, D, E, F, G, H, X)
}

export interface LipSyncResult {
  mouthCues: MouthCue[];
}

export interface RhubarbWasmModule {
  getLipSync: (pcmData: Buffer<ArrayBuffer>, dialogText?: string) => LipSyncResult;
}
