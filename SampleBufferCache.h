#pragma once
#include <utility>
class SampleInfos;
struct SampleBuffer : public juce::ReferenceCountedObject
{
    typedef juce::ReferenceCountedObjectPtr<SampleBuffer> Ptr;

    SampleBuffer(juce::File file, juce::AudioBuffer<float>&& buffer, const double d)
            : file(std::move(file)), buffer(std::move(buffer)), sourceSampleRate(d)
    {
    }

    const juce::File file;
    juce::AudioBuffer<float> buffer;
    double sourceSampleRate;
};

class SampleBufferCache
{
public:
    explicit SampleBufferCache(juce::AudioFormatManager& formatManager)
            : formatManager(formatManager) {}


    SampleBuffer::Ptr getOrCreateSampleBuffer(const SampleInfos& sampleInfos)
    {
        int position = getFilePositionIfExists(sampleInfos);
        if(position != -1)
            return sampleBuffers[position];
        return createBuffer(sampleInfos);
    }

    void checkForSamplesToFree()
    {
        for (auto i = sampleBuffers.size(); --i >= 0;)                           // [1]
        {
            SampleBuffer::Ptr buffer (sampleBuffers.getUnchecked (i)); // [2]

            if (buffer->getReferenceCount() == 2)                          // [3]
                sampleBuffers.remove (i);
        }
    }
private:


    SampleBuffer::Ptr createBuffer(const SampleInfos& sampleInfos);

    int getFilePositionIfExists(const SampleInfos& infos);


    juce::ReferenceCountedArray<SampleBuffer> sampleBuffers{};
    juce::AudioFormatManager& formatManager;
};