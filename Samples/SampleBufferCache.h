#pragma once

struct SampleBuffer : public juce::ReferenceCountedObject {
  typedef juce::ReferenceCountedObjectPtr<SampleBuffer> Ptr;

  SampleBuffer(juce::File file, juce::AudioBuffer<float> &&buffer,
               const double d)
      : file(std::move(file)), buffer(std::move(buffer)), sourceSampleRate(d) {}

  const juce::File file;
  juce::AudioBuffer<float> buffer;
  double sourceSampleRate;
};

class SampleBufferCache {
public:
  explicit SampleBufferCache(juce::AudioFormatManager &formatManager)
      : formatManager(formatManager) {}

  SampleBuffer::Ptr getOrCreateSampleBuffer(const SampleInfos &sampleInfos);

  void checkForSamplesToFree();

private:
  SampleBuffer::Ptr createBuffer(const SampleInfos &sampleInfos);

  int getFilePositionIfExists(const SampleInfos &infos);

  juce::ReferenceCountedArray<SampleBuffer> sampleBuffers{};
  juce::AudioFormatManager &formatManager;
};