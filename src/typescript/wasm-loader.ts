import { fileURLToPath } from "url";
import { dirname, join } from "path";
import { RhubarbWasmModule, LipSyncResult, MouthCue } from "./types.js";

let wasmModule: RhubarbWasmModule | null = null;

export async function initWasmModule(): Promise<RhubarbWasmModule> {
  const __filename = fileURLToPath(import.meta.url);
  const __dirname = dirname(__filename);

  const module = await import("./wasm/rhubarb.js");
  const instance = await module.default({
    locateFile: (path: string) => {
      if (path.endsWith(".wasm") || path.endsWith(".data")) {
        return join(__dirname, "wasm", path);
      }
      return path;
    },
  });

  wasmModule = instance;
  return instance;
}

export async function getLipSyncData(
  audioBase64: string,
  dialogText?: string
): Promise<LipSyncResult> {
  const module = await initWasmModule();
  const result = module.getLipSync(audioBase64, dialogText || "");
  return result;
}

export function getWasmModule(): RhubarbWasmModule | null {
  return wasmModule;
}
