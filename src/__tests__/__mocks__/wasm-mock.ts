// Mock WASM module
export const mockWasmModule = {
  getLipSync: jest.fn().mockImplementation(async (audioData: string) => {
    if (audioData === 'invalid-base64-data') {
      return Promise.reject(new Error('Invalid audio data'));
    }

    return {
      mouthCues: [
        {
          start: 0.0,
          end: 0.2,
          value: 'X'
        },
        {
          start: 0.2,
          end: 0.4,
          value: 'A'
        }
      ]
    };
  })
}; 