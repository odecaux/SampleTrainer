#pragma once
#include <lager/lenses/unbox.hpp>
#include <lager/lenses/variant.hpp>




class QuizzSource : public juce::AudioSource {


  lager::reader<Quizz::model> model;
  lager::reader<Quizz::StepType> step;
  lager::reader<size_t> step_index;
  lager::reader<std::optional<Quizz::Auditioning>> auditioning;

  SampleBufferCache &cache;
  MySampler sampler;

  Sequence sequence;
public:
  explicit QuizzSource(lager::reader<Quizz::model> model_,
                       SampleBufferCache &cache)
      : model(std::move(model_)),
        step(model[&Quizz::model::type]),
        step_index(step.xform(zug::map([](auto s) { return s.index();}))),
        auditioning(step[lager::lenses::alternative<Quizz::Auditioning>]),
        cache(cache),
        sequence(16, 16, sequence_generator())
  {
    step_index.watch([this](auto index){updateIndex(index);});
    auditioning.watch([this](auto state){ updateAuditionState(state);});

    updateIndex(step_index.get());
    updateAuditionState(auditioning.get());
  }

  void prepareToPlay(int samplesPerBlockExpected,
                     double newSampleRate) override {

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

  void updateIndex(size_t index){
    //TODO hidden dependency
    if( index == 0) {
      sequence.play();
    }
    if (index == 1) {
      auto question = std::get<Quizz::Question>(*step);
      auto samplesToPlay = sequenceSamples{
          model->kicks.get()[question.question_kick_index],
          model->snares.get()[question.question_snare_index],
          model->hats.get()[question.question_hats_index],
      };
      sequence.stop();
      swapInstruments(samplesToPlay);
      sequence.play();
    } else
      sequence.stop();
  }

  void updateAuditionState(std::optional<Quizz::Auditioning> state){
    if(state)
    {
      auto samplesToPlay = sequenceSamples{
          model->kicks.get()[state->kick_index],
          model->snares.get()[state->snare_index],
          model->hats.get()[state->hats_index]
      };
      swapInstruments(samplesToPlay);
      if(state->playing)
        sequence.play();
      else
        sequence.stop();
    }
  }

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
