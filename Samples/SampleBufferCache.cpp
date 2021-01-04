//
// Created by Octave on 13/11/2020.
//
#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>
#include "SampleInfos.h"
#include "SampleBufferCache.h"
#include "SampleRepository.h"

SampleBufferPtr SampleBufferCache::getOrCreateSampleBuffer(const SampleInfos &sampleInfos) {
  auto position  = sampleBuffers.find(sampleInfos.file);
  if(position != sampleBuffers.end())
    return position->second;

  return createBuffer(sampleInfos);
}


void SampleBufferCache::checkForSamplesToFree() {
/*  sampleBuffers.erase(std::remove_if(sampleBuffers.begin(),
                                     sampleBuffers.end(),
                                     [](const auto &item) {
                                       auto const &[key, value] = item;
                                       return value.use_count() <= 2;
                                     }),
                      sampleBuffers.end());*/
}


void SampleBufferCache::getOrCreateSampleBufferAsync(const SampleInfos &sampleInfos, const Callback& callback)
{
  auto position_samples  = sampleBuffers.find(sampleInfos.file);

  //in memory
  if(position_samples != sampleBuffers.end())
    callback(position_samples->second);

  //already loading
  auto file = sampleInfos.file.getFullPathName().toStdString();
  auto position_futures  = futures.find(file);
  if(position_samples != sampleBuffers.end())
    return;

  futures.emplace(file,
                  std::make_pair(std::async(std::launch::async,
                                            &SampleBufferCache::createBuffer,
                                            this, sampleInfos),
                                 callback));
}

SampleBufferPtr SampleBufferCache::createBuffer(const SampleInfos &sampleInfos) {
  std::unique_ptr<juce::AudioFormatReader> reader{
      formatManager.createReaderFor(sampleInfos.file)};

  if (reader == nullptr)
    return nullptr;

  auto numChannels = juce::jmin(static_cast<int>(reader->numChannels), 2);
  auto sourceSampleRate = reader->sampleRate;
  int length = juce::jmin<int>(reader->lengthInSamples, sourceSampleRate * 1000);

  jassert(length > 0);
  jassert(numChannels > 0);

  auto newBuffer = juce::AudioBuffer<float>(numChannels, length + 4);
  reader->read(&newBuffer, 0, length + 4, 0, true, true);

  auto newSampleBuffer = std::make_shared<SampleBuffer>(std::move(newBuffer), sourceSampleRate);

  sampleBuffers.insert({sampleInfos.file, newSampleBuffer});
  return newSampleBuffer;
}
