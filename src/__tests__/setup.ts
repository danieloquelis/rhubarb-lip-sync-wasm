import { Rhubarb } from '../typescript';

declare global {
  namespace NodeJS {
    interface Global {
      Module: any;
    }
  }
}

// Global setup for all tests
beforeAll(async () => {
  // Initialize the WASM module once for all tests
  await Rhubarb.getInstance();
});

// Clean up after all tests
afterAll(async () => {
  // Add any cleanup if needed
}); 