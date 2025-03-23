export interface LipSyncResult {
  phoneme: string;
  start: number;
  end: number;
}

export function getLipSync(
  audioData: Float32Array,
  sampleRate: number
): LipSyncResult[];
export default function init(): Promise<void>;
