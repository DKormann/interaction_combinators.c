
import numpy as np
import sounddevice as sd


class N:

  def __init__(self, tone: int, duration: float = 1):
    self.tone = tone
    self.duration = duration

    self.frequency = 261.63 * 2 ** (self.tone / 12)
    t = np.linspace(0, self.duration, int(22050 * self.duration))
    self.waveform = np.sin(2 * np.pi * self.frequency * t)
    self.envelope = np.exp(-2 * t)
    self.sound = (self.waveform * self.envelope * 0.3).astype(np.float32)

  def play(self):
    sd.play(self.sound, samplerate=22050)
    sd.wait()


notes = [
  N(0),
  N(1,.25),
  N(2,.75),
  N(3),
  N(0),
]




if __name__ == "__main__":
  for note in notes:
    note.play()



