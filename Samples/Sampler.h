#pragma once

// TODO migrer du midi vers un truc plus custom

class MySampler {
public:
  //==============================================================================
  MySampler() = default;

  ~MySampler() = default;

  [[nodiscard]] int getNumVoices() const noexcept { return voices.size(); }

  //==============================================================================
  [[nodiscard]] MySamplerVoice *getVoice(int index) const {
    const juce::ScopedLock sl(lock);
    return voices[index];
  }

  MySamplerVoice *addVoice(MySamplerVoice *newVoice) {
    const juce::ScopedLock sl(lock);
    newVoice->setCurrentPlaybackSampleRate(sampleRate);
    return voices.add(newVoice);
  }

  void removeVoice(int index) {
    const juce::ScopedLock sl(lock);
    voices.remove(index);
  }
  void clearVoices() {
    const juce::ScopedLock sl(lock);
    voices.clear();
  }

  //==============================================================================
  void noteOn(int midiNoteNumber) {
    const juce::ScopedLock sl(lock);

    for (auto *voice : voices) {
      if (voice->getAssignedNote() == midiNoteNumber) {
        if (voice->isVoiceActive())
          stopVoice(voice);

        startVoice(voice);
      }
    }
  }

  void allNotesOff(bool allowTailOff) {
    const juce::ScopedLock sl(lock);

    for (auto *voice : voices)
      voice->stopNote();
  }
  //==============================================================================
  void setCurrentPlaybackSampleRate(double newRate) {
    if (sampleRate != newRate) {
      const juce::ScopedLock sl(lock);
      allNotesOff(false);
      sampleRate = newRate;

      for (auto *voice : voices)
        voice->setCurrentPlaybackSampleRate(newRate);
    }
  }
  void renderNextBlock(juce::AudioBuffer<float> &outputAudio,
                       const juce::MidiBuffer &inputMidi, int startSample,
                       int numSamples) {
    processNextBlock(outputAudio, inputMidi, startSample, numSamples);
  }
  void renderNextBlock(juce::AudioBuffer<double> &outputAudio,
                       const juce::MidiBuffer &inputMidi, int startSample,
                       int numSamples) {
    processNextBlock(outputAudio, inputMidi, startSample, numSamples);
  }
  double getSampleRate() const noexcept { return sampleRate; }

  void
  setMinimumRenderingSubdivisionSize(int numSamples,
                                     bool shouldBeStrict = false) noexcept {
    minimumSubBlockSize = numSamples;
    subBlockSubdivisionIsStrict = shouldBeStrict;
  }

protected:
  juce::CriticalSection lock;

  juce::OwnedArray<MySamplerVoice> voices;

  void renderVoices(juce::AudioBuffer<float> &outputAudio, int startSample,
                    int numSamples) {
    for (auto *voice : voices)
      voice->renderNextBlock(outputAudio, startSample, numSamples);
  }

  void renderVoices(juce::AudioBuffer<double> &outputAudio, int startSample,
                    int numSamples) {
    for (auto *voice : voices)
      voice->renderNextBlock(outputAudio, startSample, numSamples);
  }

  static void startVoice(MySamplerVoice *voice) {
    if (voice != nullptr) {
      voice->startNote();
    }
  }

  static void stopVoice(MySamplerVoice *voice) {
    jassert(voice != nullptr);

    voice->stopNote();

    // the subclass MUST call clearCurrentNote() if it's not tailing off! RTFM
    // for stopNote()!
    jassert(!voice->isVoiceActive());
  }

  void handleMidiEvent(const juce::MidiMessage &m) {
    if (m.isNoteOn()) {
      noteOn(m.getNoteNumber());
    } /*
     else if (m.isNoteOff())
     {
         noteOff(m.getNoteNumber(), true);
     }
     else if (m.isAllNotesOff() || m.isAllSoundOff())
     {
         allNotesOff(true);
     }*/
  }

private:
  double sampleRate = 0;
  int minimumSubBlockSize = 32;
  bool subBlockSubdivisionIsStrict = false;

  template <typename floatType>
  void processNextBlock(juce::AudioBuffer<floatType> &outputAudio,
                        const juce::MidiBuffer &midiData, int startSample,
                        int numSamples);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySampler)
};