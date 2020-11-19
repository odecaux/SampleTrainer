#pragma once

class MySamplerVoice {
public:
  //==============================================================================
  MySamplerVoice(SampleBuffer::Ptr sample, int targetNote)
      : sample(std::move(sample)), assignedNote(targetNote) {}

  ~MySamplerVoice() = default;

  void startNote() {
    pitchRatio = sample->sourceSampleRate / getSampleRate();
    sourceSamplePosition = 0.0;
    isPlaying = true;
  }

  void stopNote() {
    sourceSamplePosition = 0.0;
    isPlaying = false;
  }

  bool isVoiceActive() const { return isPlaying; }
  int getAssignedNote() const { return assignedNote; }
  int getSamplePosition() const {
    return static_cast<int>(sourceSamplePosition);
  }

  void setCurrentPlaybackSampleRate(double newRate) {
    currentSampleRate = newRate;
  }
  double getSampleRate() const noexcept { return currentSampleRate; }

  void renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample,
                       int numSamples) {
    auto &buffer = sample->buffer;
    auto length = buffer.getNumSamples();

    if (!isVoiceActive())
      return;

    const float *const inL = buffer.getReadPointer(0);
    const float *const inR =
        buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : nullptr;

    float *outL = outputBuffer.getWritePointer(0, startSample);
    float *outR = outputBuffer.getNumChannels() > 1
                      ? outputBuffer.getWritePointer(1, startSample)
                      : nullptr;

    while (--numSamples >= 0) {
      auto pos = (int)sourceSamplePosition;

      if (sourceSamplePosition >= length) {
        stopNote();
        break;
      }
      auto alpha = (float)(sourceSamplePosition - pos);
      auto invAlpha = 1.0f - alpha;

      // just using a very simple linear interpolation here..
      float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
      float r =
          (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

      if (outR != nullptr) {
        *outL++ += 0.2f * l;
        *outR++ += 0.2f * r;
      } else {
        *outL++ += (l + r) * 0.5f;
      }

      sourceSamplePosition += pitchRatio;
    }
  }

  void renderNextBlock(juce::AudioBuffer<double> &outputBuffer, int startSample,
                       int numSamples) {
    juce::AudioBuffer<double> subBuffer(outputBuffer.getArrayOfWritePointers(),
                                        outputBuffer.getNumChannels(),
                                        startSample, numSamples);

    tempBuffer.makeCopyOf(subBuffer, true);
    renderNextBlock(tempBuffer, 0, numSamples);
    subBuffer.makeCopyOf(tempBuffer, true);
  }

private:
  //==============================================================================
  friend class MySampler;

  double currentSampleRate = 44100.0;
  double pitchRatio{};

  juce::AudioBuffer<float> tempBuffer;

  SampleBuffer::Ptr sample;

  double sourceSamplePosition = 0;
  bool isPlaying = false;
  int assignedNote;

  JUCE_LEAK_DETECTOR(MySamplerVoice)
};