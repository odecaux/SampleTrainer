//
// Created by Octave on 13/11/2020.
//
#include <juce_audio_utils/juce_audio_utils.h>
#include "SampleBufferCache.h"
#include "SampleRepository.h"
#include <memory>

SampleBuffer::Ptr SampleBufferCache::createBuffer(const SampleInfos &sampleInfos)
{
    std::unique_ptr<juce::AudioFormatReader> reader{formatManager.createReaderFor(sampleInfos.file)};

    //TODO checker que le reader existe bien

    int length = static_cast<int>(reader->lengthInSamples);
    auto numChannels = juce::jmin(static_cast<int>(reader->numChannels), 2);
    auto sourceSampleRate = reader->sampleRate;

    jassert(length > 0);
    jassert(numChannels > 0);

    auto newBuffer = juce::AudioBuffer<float>(numChannels, length + 4);
    reader->read(&newBuffer, 0, length + 4, 0, true, true);

    SampleBuffer::Ptr newSampleBuffer = new SampleBuffer{sampleInfos.file,
                                                         std::move(newBuffer),
                                                         sourceSampleRate};
    sampleBuffers.add(newSampleBuffer);
    return newSampleBuffer;
}

int SampleBufferCache::getFilePositionIfExists(const SampleInfos &infos)
{
    for(auto i = 0; i < sampleBuffers.size(); ++i)
        if(sampleBuffers[i]->file == infos.file)
            return i;
    return -1;
}