declare module '*.wasm' {
  const content: any;
  export default content;
}

declare module '*.js' {
  const content: any;
  export default content;
}

declare const Module: {
  ready: void | ((value: void) => void);
  getLipSync: (audioData: Float32Array, sampleRate: number) => Promise<Array<{
    start: number;
    end: number;
    phoneme: string;
  }>>;
}; 