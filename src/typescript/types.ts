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

export interface RhubarbWasmModule {
  getLipSync: (audioBase64: string, dialogText?: string) => LipSyncResult;
} 