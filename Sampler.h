#include <memory>
#include <utility>

using namespace juce;

class MySamplerVoice
{
public:
    //==============================================================================
    MySamplerVoice(SampleBuffer::Ptr sample,
                   int targetNote)
            : sample(std::move(sample)), assignedNote(targetNote)
    {
    }

    ~MySamplerVoice() = default;
    int getAssignedNote() const                                   { return assignedNote; }

    void startNote()                                              { samplePosition = 0.0; isPlaying = true; }
    void stopNote()                                               {
        samplePosition = 0.0; isPlaying = false ; }
    bool isVoiceActive() const                                    { return isPlaying; }
    int getSamplePosition() const                                 { return static_cast<int>(samplePosition);}

    void setCurrentPlaybackSampleRate(double newRate)             { currentSampleRate = newRate; }
    double getSampleRate() const noexcept                         { return currentSampleRate; }

    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample,  int numSamples)
    {
        auto& buffer = sample->buffer;
        auto length = buffer.getNumSamples();

        if (!isVoiceActive())
            return;

        const float* const inL = buffer.getReadPointer(0);
        const float* const inR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : nullptr;

        float* outL = outputBuffer.getWritePointer(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            auto pos = (int)samplePosition;

            if (samplePosition > length)
            {
                stopNote();
                break;
            }
            auto alpha = (float)(samplePosition - pos);
            auto invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
                                       : l;

            if (outR != nullptr)
            {
                *outL++ += 0.2f * l;
                *outR++ += 0.2f * r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            samplePosition++;

        }

    }

    void renderNextBlock (AudioBuffer<double>& outputBuffer, int startSample, int numSamples)
    {
        AudioBuffer<double> subBuffer(outputBuffer.getArrayOfWritePointers(),
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

    AudioBuffer<float> tempBuffer;

    SampleBuffer::Ptr sample;

    double samplePosition = 0;
    bool isPlaying = false;
    int assignedNote;

    JUCE_LEAK_DETECTOR (MySamplerVoice)
};

class MySampler
{
public:
    //==============================================================================
    MySampler() = default;

    ~MySampler() = default;

    void clearVoices()
    {
        const ScopedLock sl(lock);
        voices.clear();
    }

    [[nodiscard]] int getNumVoices() const noexcept                               { return voices.size(); }

    //==============================================================================
    [[nodiscard]] MySamplerVoice* getVoice(int index) const
    {
        const ScopedLock sl(lock);
        return voices[index];
    }

    MySamplerVoice* addVoice(MySamplerVoice* newVoice)
    {
        const ScopedLock sl(lock);
        newVoice->setCurrentPlaybackSampleRate(sampleRate);
        return voices.add(newVoice);
    }

    void removeVoice(int index)
    {
        const ScopedLock sl(lock);
        voices.remove(index);
    }

    //==============================================================================
    void noteOn(int midiNoteNumber)
    {
        const ScopedLock sl(lock);

        for (auto* voice : voices)
        {
            if (voice->getAssignedNote() == midiNoteNumber)
            {
                if (voice->isVoiceActive())
                    stopVoice(voice);

                startVoice(voice);
            }
        }
    }

    void noteOff (int midiNoteNumber,
                  bool allowTailOff)
    {
        const ScopedLock sl(lock);

        for (auto* voice : voices)
        {
            if (voice->isVoiceActive() && midiNoteNumber == voice->getAssignedNote())
            {
                //stopVoice (voice);
            }
        }
    }

    void allNotesOff(bool allowTailOff)
    {
        const ScopedLock sl(lock);

        for (auto* voice : voices)
            voice->stopNote();
    }
    //==============================================================================
    void setCurrentPlaybackSampleRate(double newRate)
    {
        if (sampleRate != newRate)
        {
            const ScopedLock sl(lock);
            allNotesOff(false);
            sampleRate = newRate;

            for (auto* voice : voices)
                voice->setCurrentPlaybackSampleRate(newRate);
        }
    }
    void renderNextBlock (AudioBuffer<float>& outputAudio,
                          const MidiBuffer& inputMidi,
                          int startSample,
                          int numSamples)
    {
        processNextBlock(outputAudio, inputMidi, startSample, numSamples);
    }
    void renderNextBlock (AudioBuffer<double>& outputAudio,
                          const MidiBuffer& inputMidi,
                          int startSample,
                          int numSamples)
    {
        processNextBlock(outputAudio, inputMidi, startSample, numSamples);
    }
    double getSampleRate() const noexcept                       { return sampleRate; }

    void setMinimumRenderingSubdivisionSize(int numSamples, bool shouldBeStrict = false) noexcept
    {
        minimumSubBlockSize = numSamples;
        subBlockSubdivisionIsStrict = shouldBeStrict;
    }

protected:
    CriticalSection lock;

    OwnedArray<MySamplerVoice> voices;

    void renderVoices (AudioBuffer<float>& outputAudio,
                       int startSample, int numSamples)
    {
        for (auto* voice : voices)
            voice->renderNextBlock(outputAudio, startSample, numSamples);
    }

    void renderVoices (AudioBuffer<double>& outputAudio,
                       int startSample, int numSamples)
    {
        for (auto* voice : voices)
            voice->renderNextBlock(outputAudio, startSample, numSamples);
    }

    static void startVoice(MySamplerVoice* voice)
    {
        if (voice != nullptr)
        {
            voice->startNote();
        }
    }

    static void stopVoice(MySamplerVoice* voice)
    {
        jassert(voice != nullptr);

                        voice->stopNote();

                        // the subclass MUST call clearCurrentNote() if it's not tailing off! RTFM for stopNote()!
                        jassert(!voice->isVoiceActive());
                                    }

    void handleMidiEvent(const MidiMessage& m)
    {
        if (m.isNoteOn())
        {
            noteOn(m.getNoteNumber());
        }/*
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
    void processNextBlock(AudioBuffer<floatType>& outputAudio, const MidiBuffer& midiData, int startSample, int numSamples)
    {
        // must set the sample rate before using this!
        jassert(sampleRate != 0);
        const int targetChannels = outputAudio.getNumChannels();

        auto midiIterator = midiData.findNextSamplePosition(startSample);

        bool firstEvent = true;

        const ScopedLock sl(lock);

        for (; numSamples > 0; ++midiIterator)
        {
            if (midiIterator == midiData.cend())
            {
                if (targetChannels > 0)
                    renderVoices(outputAudio, startSample, numSamples);

                return;
            }

            const auto metadata = *midiIterator;
            const int samplesToNextMidiMessage = metadata.samplePosition - startSample;

            if (samplesToNextMidiMessage >= numSamples)
            {
                if (targetChannels > 0)
                    renderVoices(outputAudio, startSample, numSamples);

                handleMidiEvent(metadata.getMessage());
                break;
            }

            if (samplesToNextMidiMessage < ((firstEvent && !subBlockSubdivisionIsStrict) ? 1 : minimumSubBlockSize))
            {
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

        std::for_each(midiIterator,
                      midiData.cend(),
                      [&](const MidiMessageMetadata& meta) { handleMidiEvent(meta.getMessage()); });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySampler)
};