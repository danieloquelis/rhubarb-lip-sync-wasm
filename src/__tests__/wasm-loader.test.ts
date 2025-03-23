import { initWasmModule, getWasmModule } from '../typescript/wasm-loader';

declare global {
  namespace NodeJS {
    interface Global {
      Module: any;
    }
  }
}

describe('WASM Loader', () => {
  it('should initialize WASM module', async () => {
    const module = await initWasmModule();
    expect(module).toBeDefined();
    expect(module.getLipSync).toBeDefined();
    expect(typeof module.getLipSync).toBe('function');
  });

  it('should return the same module instance on subsequent calls', async () => {
    const module1 = await initWasmModule();
    const module2 = await initWasmModule();
    expect(module1).toBe(module2);
  });

  it('should get the current module instance', () => {
    const module = getWasmModule();
    expect(module).toBeDefined();
  });

  it('should handle invalid audio data', async () => {
    const module = await initWasmModule();
    await expect(
      module.getLipSync('invalid-base64-data', '')
    ).rejects.toThrow('Invalid audio data');
  });
}); 