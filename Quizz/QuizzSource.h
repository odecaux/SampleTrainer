#pragma once

class QuizzSource : public juce::AudioSource {
public:
  QuizzSource() {
    std::vector<Note> notes{
        {kick, 0},  {snare, 4},
        {kick, 7},  {kick, 8},  {snare, 12}};

    //on génère des 16th de hats
    std::vector<Note> hats(16);
    std::ranges::generate(hats, [n = 0]()mutable { return Note{SampleType::hats, n++}; });
    std::ranges::copy(hats, std::back_inserter(notes));

    sequence = std::make_unique<Sequence>(Sequence{16, 16, std::move(notes)});
  }
  void prepareToPlay(int samplesPerBlockExpected,
                     double newSampleRate) override {
    sampleRate = newSampleRate;
    numSamples = samplesPerBlockExpected;

    sequence->prepareToPlay(samplesPerBlockExpected, newSampleRate);
    sampler.setCurrentPlaybackSampleRate(newSampleRate);
    midiCollector.reset(newSampleRate); // [10]
  }

  void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override {
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer midiMessages;
    // midiCollector.removeNextBlockOfMessages (midiMessages,
    // bufferToFill.numSamples); // [11]

    sequence->renderNextBlock(midiMessages);
    sampler.renderNextBlock(*bufferToFill.buffer, midiMessages,
                            bufferToFill.startSample, bufferToFill.numSamples);
  }

  void releaseResources() override {}

  void addSample(const SampleInfos &sampleInfos, SampleBufferCache &cache) {
    auto buffer = cache.getOrCreateSampleBuffer(sampleInfos);
    sampler.addVoice(new MySamplerVoice(buffer, 60 + sampleInfos.type));
    mSampleCount++;
  }

  void clearSamples() { sampler.clearVoices(); }

  juce::MidiMessageCollector &getMidiCollector() { return midiCollector; }

  void play() { sequence->play(); }
  void stop() { sequence->stop(); }

private:
  juce::MidiMessageCollector midiCollector;
  MySampler sampler;

  std::unique_ptr<Sequence> sequence;
  int numSamples{};
  double sampleRate{};
  int mSampleCount{};
  int time{};
};
