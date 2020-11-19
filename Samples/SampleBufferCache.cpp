//
// Created by Octave on 13/11/2020.
//
#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>
#include "SampleInfos.h"
#include "SampleBufferCache.h"
#include "SampleRepository.h"

SampleBuffer::Ptr
SampleBufferCache::createBuffer(const SampleInfos &sampleInfos) {
  std::unique_ptr<juce::AudioFormatReader> reader{
      formatManager.createReaderFor(sampleInfos.file)};

  if (reader == nullptr)
    return nullptr;

  auto numChannels = juce::jmin(static_cast<int>(reader->numChannels), 2);
  auto sourceSampleRate = reader->sampleRate;
  int length = juce::jmin<int>(reader->lengthInSamples, sourceSampleRate * 5);

  jassert(length > 0);
  jassert(numChannels > 0);

  auto newBuffer = juce::AudioBuffer<float>(numChannels, length + 4);
  reader->read(&newBuffer, 0, length + 4, 0, true, true);

  SampleBuffer::Ptr newSampleBuffer = new SampleBuffer{
      sampleInfos.file, std::move(newBuffer), sourceSampleRate};

  sampleBuffers.add(newSampleBuffer);
  return newSampleBuffer;
}

int SampleBufferCache::getFilePositionIfExists(const SampleInfos &infos) {
  for (auto i = 0; i < sampleBuffers.size(); ++i)
    if (sampleBuffers[i]->file == infos.file)
      return i;
  return -1;
}

SampleBuffer::Ptr SampleBufferCache::getOrCreateSampleBuffer(const SampleInfos &sampleInfos) {
  int position = getFilePositionIfExists(sampleInfos);
  if (position != -1)
    return sampleBuffers[position];
  return createBuffer(sampleInfos);
}

void SampleBufferCache::checkForSamplesToFree() {
  for (auto i = sampleBuffers.size(); --i >= 0;) // [1]
  {
    SampleBuffer::Ptr buffer(sampleBuffers.getUnchecked(i)); // [2]

    if (buffer->getReferenceCount() == 2) // [3]
      sampleBuffers.remove(i);
  }
}