#pragma once
#include <lager/lenses/unbox.hpp>




class QuizzSource : public juce::AudioSource {

  lager::reader<Quizz::model> model;
  lager::reader<Quizz::StepType> step;

  SampleBufferCache &cache;
  MySampler sampler;

  Sequence sequence;

  int numSamples{};
  double sampleRate{};
  int time{};
public:
  explicit QuizzSource(lager::reader<Quizz::model> model_,
                       SampleBufferCache &cache)
      : model(model_),
        step(model_[&Quizz::model::type]),
        cache(cache),
        sequence(16, 16, sequence_generator())
  {
    //TODO reecrire tout ça, c'est super nul
    watch(step, [this](Quizz::StepType newStep){
      if(std::holds_alternative<Quizz::Auditioning>(newStep))
      {
        auto samplesToPlay = sequenceSamples{
            model->kicks.samples.get()[model->kicks.selected_index.value_or(0)],
            model->snares.samples.get()[model->snares.selected_index.value_or(0)],
            model->hats.samples.get()[model->hats.selected_index.value_or(0)],
        };
        swapInstruments(samplesToPlay);
        sequence.play();
      }
      if(std::holds_alternative<Quizz::Question>(newStep))
      {
        auto question = std::get<Quizz::Question>(newStep);
        auto samplesToPlay = sequenceSamples{
            model->kicks.samples.get()[question.kick_index],
            model->snares.samples.get()[question.snare_index],
            model->hats.samples.get()[question.hats_index],
        };
        sequence.stop();
        swapInstruments(samplesToPlay);
        sequence.play();
      }
      else
        sequence.stop();
    });

  }

  void prepareToPlay(int samplesPerBlockExpected,
                     double newSampleRate) override {
    sampleRate = newSampleRate;
    numSamples = samplesPerBlockExpected;

    sequence.prepareToPlay(samplesPerBlockExpected, newSampleRate);
    sampler.setCurrentPlaybackSampleRate(newSampleRate);
  }

  void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override {
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer midiMessages;

    sequence.renderNextBlock(midiMessages);
    sampler.renderNextBlock(*bufferToFill.buffer, midiMessages,
                            bufferToFill.startSample, bufferToFill.numSamples);
  }

  void releaseResources() override {}


private:

  void swapInstruments(const sequenceSamples& selectedSamples)
  {
    sampler.clearVoices();
    setSamples(selectedSamples.hats);
    setSamples(selectedSamples.snare);
    setSamples(selectedSamples.kick);
  }

  void setSamples(const SampleInfos &sampleInfos) {
    auto buffer = cache.getOrCreateSampleBuffer(sampleInfos);
    sampler.addVoice(new MySamplerVoice(buffer, 60 + sampleInfos.type));
  }


  void play() { sequence.play(); }
  void stop() { sequence.stop(); }

  static std::vector<Note> sequence_generator()
  {
      std::vector<Note> notes{
          {kick, 0},  {snare, 4},
          {kick, 7},  {kick, 8},  {snare, 12}};

      //on génère des 16th de hats
      std::vector<Note> hats(16);
      std::generate(hats.begin(), hats.end(), [n = 0]()mutable { return Note{SampleType::hats, n++}; });
      std::copy(hats.begin(), hats.end(), std::back_inserter(notes));
      return notes;
  }
};


class QuizzPlayer{
  juce::AudioDeviceManager &audioDeviceManager;
  juce::AudioSourcePlayer audioSourcePlayer;
  QuizzSource source;
public:
  QuizzPlayer(lager::reader<Quizz::model> model_, SampleBufferCache &cache,
              juce::AudioDeviceManager &dm)
      : audioDeviceManager(dm),
        source(std::move(model_), cache)
  {
    audioDeviceManager.addAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(&source);
  }

  ~QuizzPlayer() {
    audioSourcePlayer.setSource(nullptr);
    audioDeviceManager.removeAudioCallback(&audioSourcePlayer);
  }
};
