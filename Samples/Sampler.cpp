//
// Created by Octave on 19/11/2020.
//
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include "SampleInfos.h"
#include "SampleBufferCache.h"
#include "SampleVoice.h"
#include "Sampler.h"

template void MySampler::processNextBlock<float>  (juce::AudioBuffer<float>&,  const juce::MidiBuffer&, int, int);
template void MySampler::processNextBlock<double> (juce::AudioBuffer<double>&, const juce::MidiBuffer&, int, int);
template <typename floatType>
void MySampler::processNextBlock(juce::AudioBuffer<floatType> &outputAudio,
                                 const juce::MidiBuffer &midiData,
                                 int startSample, int numSamples) {
  // must set the sample rate before using this!
  jassert(sampleRate != 0);
  const int targetChannels = outputAudio.getNumChannels();

  auto midiIterator = midiData.findNextSamplePosition(startSample);

  bool firstEvent = true;

  const juce::ScopedLock sl(lock);

  for (; numSamples > 0; ++midiIterator) {
    if (midiIterator == midiData.cend()) {
      if (targetChannels > 0)
        renderVoices(outputAudio, startSample, numSamples);

      return;
    }

    const auto metadata = *midiIterator;
    const int samplesToNextMidiMessage = metadata.samplePosition - startSample;

    if (samplesToNextMidiMessage >= numSamples) {
      if (targetChannels > 0)
        renderVoices(outputAudio, startSample, numSamples);

      handleMidiEvent(metadata.getMessage());
      break;
    }

    if (samplesToNextMidiMessage < ((firstEvent && !subBlockSubdivisionIsStrict)
                                        ? 1
                                        : minimumSubBlockSize)) {
      handleMidiEvent(metadata.getMessage());
      continue;
    }

    firstEvent = false;

    if (targetChannels > 0)
      renderVoices(outputAudio, startSample, samplesToNextMidiMessage);

    handleMidiEvent(metadata.getMessage());
    startSample += samplesToNextMidiMessage;
    numSamples -= samplesToNextMidiMessage;
  }

  std::for_each(midiIterator, midiData.cend(),
                [&](const juce::MidiMessageMetadata &meta) {
                  handleMidiEvent(meta.getMessage());
                });

  std::for_each(outputAudio.getReadPointer(0),
                outputAudio.getReadPointer(0) + numSamples,
                [](float s) { jassert(s < 1); });
}