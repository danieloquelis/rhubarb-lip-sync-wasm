import { RhubarbWasmModule } from "./types";

export function initWasmModule(): Promise<RhubarbWasmModule>;
export function getWasmModule(): RhubarbWasmModule | null;
