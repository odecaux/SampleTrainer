#pragma once
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/comparison.hpp>

struct Note {
  SampleType type;
  int position;
};


struct sequenceSamples{
  SampleInfos kick;
  SampleInfos snare;
  SampleInfos hats;
};

BOOST_FUSION_ADAPT_STRUCT(sequenceSamples, kick, snare, hats);
using boost::fusion::operator==;
using boost::fusion::operator!=;

struct Sequence {
  Sequence(int subdivision, int length, std::vector<Note> &&notes)
      : subdivision(subdivision), length(length), notes(std::move(notes)) {}

  int subdivision;
  int length;
  std::vector<Note> notes;

  void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) {
    numSamples = samplesPerBlockExpected;
    sampleRate = newSampleRate;
  }

  void renderNextBlock(juce::MidiBuffer &midiMessages) {
    if (!isPlaying)
      return;

    int tempo = 85;
    double quarter = 60.0 / tempo;
    auto interval = static_cast<int>(quarter * sampleRate * 4 / subdivision);
    auto sequence_length = length * interval;
    auto reste = time + numSamples - sequence_length;

    for (auto const &note : notes)
    {
      auto beatPlacement = note.position * interval;


      auto is_in_this_window = beatPlacement >= time && beatPlacement < time + numSamples;
      auto is_in_wrapped_remain = reste > 0 && beatPlacement < reste;
      auto should_play = is_in_this_window || is_in_wrapped_remain;

      if(should_play)
      {
        int timing;
        if (is_in_this_window)
          timing = beatPlacement - time;
        else
          timing = beatPlacement + (sequence_length - time);

        midiMessages.addEvent(
            juce::MidiMessage::noteOn(1, 60 + note.type, (juce::uint8)127),
            timing);
        midiMessages.addEvent(
            juce::MidiMessage::noteOff(1, 60 + note.type, 0.0f),
            timing + 10);
      }
    }
    time = (time + numSamples) % sequence_length;
  }

  void play() {
    isPlaying = true;
  }

  void stop() {
    isPlaying = false;
    time = 0;
  }

private:
  bool isPlaying{false};
  int time{};
  int numSamples{};
  double sampleRate{};
};