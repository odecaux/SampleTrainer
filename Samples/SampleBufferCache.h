#pragma once

struct SampleBuffer {
  SampleBuffer(juce::AudioBuffer<float> &&buffer,
               const double d)
      : buffer(std::move(buffer)), sourceSampleRate(d) {}

  juce::AudioBuffer<float> buffer;
  double sourceSampleRate;
};


typedef std::shared_ptr<SampleBuffer> SampleBufferPtr;
//TODO mettre des std::optional

class SampleBufferCache {
public:
  explicit SampleBufferCache(juce::AudioFormatManager &formatManager)
      : formatManager(formatManager) {}

  SampleBufferPtr getOrCreateSampleBuffer(const SampleInfos &sampleInfos);

  void checkForSamplesToFree();

private:
  SampleBufferPtr createBuffer(const SampleInfos &sampleInfos);

  std::map<juce::File, SampleBufferPtr> sampleBuffers{};
  juce::AudioFormatManager &formatManager;
};