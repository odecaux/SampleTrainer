#pragma once

struct Note
{
    SampleType type;
    int position;
};

struct Sequence
{
    Sequence(int subdivision, int length, std::vector<Note>&& notes)
    : subdivision(subdivision), length(length), notes(std::move(notes))
    {}

    int subdivision;
    int length;
    std::vector<Note> notes;

    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate)
    {
        numSamples = samplesPerBlockExpected;
        sampleRate = newSampleRate;
    }

    void renderNextBlock(juce::MidiBuffer& midiMessages)
    {
        if(!isPlaying)
            return;
        int tempo = 85;
        double quarter = 60.0/tempo;
        auto interval = static_cast<int>(quarter * sampleRate * 4 / subdivision);
        auto sequence_length = length * interval;
        auto reste = time + numSamples - sequence_length;

        for(auto const& note : notes)
        {
            auto beatPlacement = note.position * interval;
            //for each beat in list
            if(beatPlacement >= time && beatPlacement < time + numSamples)
            {
                auto timing = beatPlacement - time;
                midiMessages.addEvent(
                        juce::MidiMessage::noteOn(1, 60 + note.type, (juce::uint8) 127),
                        timing);
                midiMessages.addEvent(
                        juce::MidiMessage::noteOff(1, 60+ note.type, 0.0f),
                        timing + 100);
            }
            else if(( reste > 0 && beatPlacement < reste ) )
            {
                auto timing = beatPlacement + (sequence_length - time);
                midiMessages.addEvent(
                        juce::MidiMessage::noteOn(1, 60+ note.type, (juce::uint8) 127),
                        timing);
                midiMessages.addEvent(
                        juce::MidiMessage::noteOff(1, 60+ note.type, 0.0f),
                        timing + 10);
            }
        }


        time = (time + numSamples) % sequence_length;

    }

    void play()
    {
        isPlaying = true;
    }

    void stop()
    {
        isPlaying = false;
        time = 0;
    }
private:
    bool isPlaying{false};
    int time{};
    int numSamples{};
    double sampleRate{};
};