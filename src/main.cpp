#include "Tonic.h"
#include "Tonic/Generator.h"
#include "Tonic/SineWave.h"
#include "Tonic/TriangleWave.h"
#include "rtaudio/RtAudio.h"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace Tonic;
using namespace std;

const unsigned int nChannels = 2;

static Synth synth;

int renderCallback(void *outputBuffer, void *inputBuffer,
                   unsigned int nBufferFrames, double streamTime,
                   RtAudioStreamStatus status, void *userData) {
  synth.fillBufferOfFloats((float *)outputBuffer, nBufferFrames, nChannels);
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

  Generator basstone = SineWave().freq(50);
  Generator thing = TriangleWave().freq(80);
  SineWave lfo = SineWave().freq(0.1);
  TriangleWave anotherLfo = TriangleWave().freq(0.4);

  Generator output = (basstone * anotherLfo) + (thing * lfo);

  synth.setOutputGen(output);

  // ---------------------------------------

  dac.openStream(&rtParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames,
                 &renderCallback, NULL, NULL);

  dac.startStream();

  // hacky, yes, but let's just hang out for awhile until someone presses a key
  printf("\n\nPress Enter to stop\n\n");
  cin.get();

  dac.stopStream();

  return 0;
}
