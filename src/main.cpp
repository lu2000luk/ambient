#include "Tonic.h"
#include "Tonic/CompressorLimiter.h"
#include "Tonic/ControlRandom.h"
#include "Tonic/Filters.h"
#include "Tonic/Generator.h"
#include "Tonic/LFNoise.h"
#include "Tonic/Noise.h"
#include "Tonic/Reverb.h"
#include "Tonic/SawtoothWave.h"
#include "Tonic/SineWave.h"
#include "Tonic/TriangleWave.h"
#include "rtaudio/RtAudio.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace Tonic;
using namespace std;

const unsigned int nChannels = 2;

int getTimestamp() {
  return chrono::duration_cast<chrono::milliseconds>(
             chrono::system_clock::now().time_since_epoch())
      .count();
}

static Synth synth;
static int crossfade_start = 0;
static bool in_crossfade = false;
static int last_crossfade = getTimestamp();

static Generator current_synth;
static Generator next_synth;

Generator synth1() {
  ControlRandom random = ControlRandom().min(60).max(90);
  Generator basstone = SineWave().freq(random);
  Generator thing = TriangleWave().freq(random);
  SineWave lfo = SineWave().freq(0.2);
  SawtoothWave anotherLfo = SawtoothWave().freq(0.4);

  Generator output = ((basstone * anotherLfo * 0.2) + (thing * lfo * 0.2)) +
                     (LPF24().cutoff(2000).input(PinkNoise()) * 0.005);
  return output;
}

Generator synth2() {
  Generator output =
      (LPF24()
           .cutoff((SineWave().freq(1) * 800) + 0.5)
           .Q(2)
           .input(RectWave().freq(SineWave().freq(0.5) * 80)) *
       0.5) +
      (LPF24()
           .cutoff((SineWave().freq(2) * 1000))
           .Q(1)
           .input(TriangleWave().freq(SineWave().freq(1) * 80 + 100)) *
       0.5) +
      SineWave().freq(60) * SineWave().freq(2);
  return output;
}

Generator synth3() {
  ControlRandom random = ControlRandom().min(0).max(50);
  Generator output =
      Compressor().ratio(8).input(
          LPF24().cutoff(400).normalizesGain(true).input(
              SawtoothWave().freq(TriangleWave().freq(0.1) * (150 + random))) *
              -1 +
          LPF24().cutoff(700).normalizesGain(true).input(
              TriangleWave().freq(SineWave().freq(1.6) * (80 + random))) *
              TriangleWave().freq(0.5) * -1 +
          LPF24().cutoff(800).normalizesGain(true).input(
              RectWave().freq(SawtoothWave().freq(0.4) * (200 + random))) *
              -1) *
          0.5 +
      SineWave().freq(40) * 0.1;

  return output;
}

Generator synth4() {
  // Deep sub-bass with slight detuning for a warm, beating drone
  ControlRandom random = ControlRandom().min(30).max(55);
  Generator sub = SineWave().freq(random);
  Generator pad = TriangleWave().freq(
      random * 1.005); // Slight detune creates phase beating
  SineWave lfo1 = SineWave().freq(0.12);
  SineWave lfo2 = SineWave().freq(0.07);

  // Filtered noise simulates distant ocean waves
  Generator noiseWash =
      LPF24().cutoff(400 + lfo2 * 300).Q(1.5).input(PinkNoise()) * 0.008;

  Generator output = (sub * 0.25) + (pad * lfo1 * 0.2) + noiseWash;
  return output;
}

Generator synth5() {
  // Soft sawtooth pad that swells and recedes very slowly
  ControlRandom random = ControlRandom().min(50).max(80);
  Generator osc1 = SawtoothWave().freq(random);
  Generator osc2 = SawtoothWave().freq(random * 0.998); // Slight chorus effect
  SineWave lfo = SineWave().freq(0.25);

  // Keep the filter cutoff very low to maintain a muffled, warm texture
  Generator filtered =
      LPF24().cutoff(300 + lfo * 250).Q(3.5).input(osc1 + osc2);

  Generator output = (filtered * SineWave().freq(0.08) * 0.3) +
                     (SineWave().freq(random * 0.5) * 0.08); // Sub octave
  return output;
}

Generator synth6() {
  // Fluffy square wave drift with a slow tremolo
  ControlRandom random = ControlRandom().min(40).max(65);
  Generator pulse = RectWave().freq(random);
  TriangleWave lfo1 = TriangleWave().freq(0.1); // Very slow sweep
  SineWave lfo2 = SineWave().freq(0.18);        // Tremolo

  Generator filteredPulse =
      LPF24().cutoff(500 + lfo1 * 400).Q(2.0).input(pulse);
  Generator sub = SineWave().freq(random * 0.5) * lfo2 * 0.12;

  Generator output = (filteredPulse * 0.18) + sub;
  return output;
}

Generator synth7() {
  // Gentle FM modulation creating a soft, bell-like but deeply pitched tone
  ControlRandom random = ControlRandom().min(55).max(70);

  // Slow modulator creates an evolving harmonic texture
  SineWave modulator = SineWave().freq(SineWave().freq(0.1) * 15);
  Generator carrier = SineWave().freq(random + modulator * 8);
  TriangleWave lfo = TriangleWave().freq(0.13);

  Generator filtered = LPF24().cutoff(750).Q(1.0).input(carrier);

  Generator output =
      (filtered * lfo * 0.35) + (SineWave().freq(38) * 0.08); // Root drone
  return output;
}

Generator synth8() {
  // Layered sub-harmonics glued together with a compressor
  ControlRandom random = ControlRandom().min(30).max(55);

  Generator tone1 = SineWave().freq(random) * SineWave().freq(0.09);
  Generator tone2 =
      TriangleWave().freq(random * 1.5) * SineWave().freq(0.14); // Fifth
  Generator tone3 =
      SineWave().freq(random * 2.0) * TriangleWave().freq(0.19); // Octave

  Generator mix = (tone1 * 0.3) + (tone2 * 0.2) + (tone3 * 0.15);

  Generator output =
      Compressor().ratio(4).input(
          LPF24().cutoff(600 + SineWave().freq(0.2) * 250).input(mix)) *
      0.5;

  return output;
}

int renderCallback(void *outputBuffer, void *inputBuffer,
                   unsigned int nBufferFrames, double streamTime,
                   RtAudioStreamStatus status, void *userData) {
  synth.fillBufferOfFloats((float *)outputBuffer, nBufferFrames, nChannels);

  if (in_crossfade) {
    if (getTimestamp() - crossfade_start >= 5000) {
      in_crossfade = false;
      crossfade_start = 0;
      cout << "crossfade end" << endl;
      synth.setOutputGen(next_synth * 0.8);

      // get random value 1-8
      int randomValue = rand() % 8 + 1;
      cout << "next synth: " << randomValue << endl;

      current_synth = next_synth;

      if (randomValue == 1) {
        next_synth = synth1();
      } else if (randomValue == 2) {
        next_synth = synth2();
      } else if (randomValue == 3) {
        next_synth = synth3();
      } else if (randomValue == 4) {
        next_synth = synth4();
      } else if (randomValue == 5) {
        next_synth = synth5();
      } else if (randomValue == 6) {
        next_synth = synth6();
      } else if (randomValue == 7) {
        next_synth = synth7();
      } else if (randomValue == 8) {
        next_synth = synth8();
      }
    }
  }

  if (getTimestamp() - last_crossfade >= 20000) {
    in_crossfade = true;
    last_crossfade = getTimestamp();
    crossfade_start = getTimestamp();
    cout << "crossfade start" << endl;

    SineWave lfo = SineWave().freq(0.025);
    Generator crossfade = current_synth * (1.0 - lfo) + next_synth * lfo;
    synth.setOutputGen(crossfade * 0.8);
  }

  return 0;
}

int main(int argc, const char *argv[]) {
  // Configure RtAudio
  RtAudio dac;
  RtAudio::StreamParameters rtParams;
  rtParams.deviceId = dac.getDefaultOutputDevice();

  cout << dac.getDefaultOutputDevice() << endl;

  std::vector<unsigned int> deviceIds = dac.getDeviceIds();

  if (deviceIds.empty()) {
    std::cout << "No device." << std::endl;
    return 0;
  }

  for (unsigned int id : deviceIds) {
    try {
      RtAudio::DeviceInfo info = dac.getDeviceInfo(id);

      std::cout << "ID: " << id << "\n";
      std::cout << "Nome: " << info.name << "\n\n";
    } catch (RtAudioErrorType &e) {
      std::cerr << "Error " << id << ": " << e << std::endl;
    }
  }

  rtParams.nChannels = nChannels;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = 512; // 512 sample frames

  Tonic::setSampleRate(sampleRate);

  // --------- SYNTH -----------------------

  current_synth = synth3();
  next_synth = synth2();
  synth.setOutputGen(current_synth * 0.8);

  // ---------------------------------------

  dac.openStream(&rtParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames,
                 &renderCallback, NULL, NULL);

  dac.startStream();

  printf("\n\nPress Enter to stop\n");
  cin.get();

  dac.stopStream();

  return 0;
}
