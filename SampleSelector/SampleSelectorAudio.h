#pragma once

class SampleSelectorAudio
{
public:
    SampleSelectorAudio(juce::AudioDeviceManager& dm,
                        SampleBufferCache& sl)
            : audioDeviceManager(dm), sampleLoader(sl)
    {
        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);
    }

    ~SampleSelectorAudio()
    {
        transportSource  .setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);
        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
    }

    void playSound(const SampleInfos& sampleInfos)
    {

        loadURLIntoTransport(sampleInfos);
    }

    void stop()
    {
        transportSource.stop();
        transportSource.setPosition(0);
    }

    void stopAndRelease()
    {
        stop();
        transportSource.setSource(nullptr);
        memorySource.reset();
    }


    void loadURLIntoTransport (const SampleInfos& sampleInfos)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        memorySource.reset();

        //TODO faire une meilleure gestion des erreurs
        auto sample = sampleLoader.getOrCreateSampleBuffer(sampleInfos);

        memorySource = std::make_unique<juce::MemoryAudioSource>(sample->buffer, false, false);


        transportSource.setSource(memorySource.get(),
                                  0,
                                  nullptr,
                                  sample->sourceSampleRate,
                                  sample->buffer.getNumChannels());


        transportSource.setPosition(0);
        transportSource.start();
    }

private:


    juce::AudioDeviceManager& audioDeviceManager;

    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::MemoryAudioSource> memorySource;

    SampleBufferCache& sampleLoader;
};