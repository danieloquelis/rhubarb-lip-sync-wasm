import { Rhubarb } from '../typescript';
import fs from 'fs';
import path from 'path';

describe('Rhubarb Lip Sync', () => {
  let testAudioBase64: string;

  beforeAll(() => {
    // Load test audio file
    const audioPath = path.join(__dirname, 'fixtures', 'test.wav');
    const audioBuffer = fs.readFileSync(audioPath);
    testAudioBase64 = audioBuffer.toString('base64');
  });

  it('should initialize successfully', async () => {
    const instance = await Rhubarb.getInstance();
    expect(instance).toBeDefined();
  });

  it('should generate lip sync data from audio', async () => {
    const result = await Rhubarb.getLipSync(testAudioBase64);
    
    expect(result).toBeDefined();
    expect(result.mouthCues).toBeInstanceOf(Array);
    expect(result.mouthCues.length).toBeGreaterThan(0);
    
    // Check mouth cue structure
    const firstCue = result.mouthCues[0];
    expect(firstCue).toHaveProperty('start');
    expect(firstCue).toHaveProperty('end');
    expect(firstCue).toHaveProperty('value');
    
    // Validate timing
    expect(firstCue.start).toBeGreaterThanOrEqual(0);
    expect(firstCue.end).toBeGreaterThan(firstCue.start);
  });

  it('should handle different recognizer types', async () => {
    const result = await Rhubarb.getLipSync(testAudioBase64, {
      recognizerType: 'phonetic'
    });
    
    expect(result).toBeDefined();
    expect(result.mouthCues).toBeInstanceOf(Array);
  });

  it('should handle dialog text option', async () => {
    const result = await Rhubarb.getLipSync(testAudioBase64, {
      dialogText: 'Test dialog text'
    });
    
    expect(result).toBeDefined();
    expect(result.mouthCues).toBeInstanceOf(Array);
  });

  it('should throw error for invalid audio data', async () => {
    await expect(
      Rhubarb.getLipSync('invalid-base64-data')
    ).rejects.toThrow();
  });
}); 