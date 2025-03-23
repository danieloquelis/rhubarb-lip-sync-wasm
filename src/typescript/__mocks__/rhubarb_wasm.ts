export default jest.fn().mockResolvedValue(undefined);

export const getLipSync = jest.fn().mockReturnValue([
  {
    phoneme: 'A',
    start: 0.0,
    end: 0.1,
  },
]); 