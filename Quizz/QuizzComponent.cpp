
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_gui_extra/juce_gui_extra.h"

#include "../Samples/SampleInfos.h"
#include "../Samples/SampleBufferCache.h"
#include "SampleVoice.h"
#include "Sampler.h"
#include "Sequencer.h"

#include "QuizzSource.h"

#include "Game.h"
#include "QuizzComponent.h"



QuizzComponent::QuizzComponent(juce::AudioDeviceManager &dm,
                                        SampleBufferCache &cache)
    : deviceManager(dm), kickModel(*this), snareModel(*this), hatsModel(*this),
      cache(cache) {
  kickList.setModel(&kickModel);
  snareList.setModel(&snareModel);
  hatsList.setModel(&hatsModel);

  addAndMakeVisible(kickList);
  addAndMakeVisible(snareList);
  addAndMakeVisible(hatsList);
  addAndMakeVisible(answerButton);

  deviceManager.addAudioCallback(&audioSourcePlayer);
  audioSourcePlayer.setSource(&samplerSource);

  setupMidi();
}

QuizzComponent::~QuizzComponent() {
  audioSourcePlayer.setSource(nullptr);
  deviceManager.removeAudioCallback(&audioSourcePlayer);
}

void QuizzComponent::resized() {
  auto bounds = getLocalBounds().reduced(10);

  int bottomHeight = 40;

  auto listWidth = bounds.getWidth() / 3;
  auto listBound = bounds.withTrimmedBottom(bottomHeight).withWidth(listWidth);

  kickList.setBounds(listBound.reduced(3));
  snareList.setBounds((listBound + juce::Point(listWidth, 0)).reduced(3));
  hatsList.setBounds((listBound + juce::Point(listWidth * 2, 0)).reduced(3));

  auto buttonRow = bounds.withTrimmedTop(listBound.getHeight()).reduced(3);
  answerButton.setBounds(buttonRow.withTrimmedLeft(buttonRow.getWidth() - 120));
}

void QuizzComponent::addSamples(std::vector<SampleInfos> &&samples) {
  for (SampleInfos &sample : samples) {
    if (sample.type == SampleType::kick) {
      kickModel.addSample(sample);
      kickList.updateContent();
    } else if (sample.type == SampleType::snare) {
      snareModel.addSample(sample);
      snareList.updateContent();
    } else if (sample.type == SampleType::hats) {
      hatsModel.addSample(sample);
      hatsList.updateContent();
    }
  }

  initGame();
}

void QuizzComponent::initGame() {
  answerButton.onClick = [this] { checkAnswers(); };
  nextQuestion();
}

void QuizzComponent::checkAnswers() {
  stop();
  if (usedHats && usedKick && usedSnare) {
    if (*usedHats == hatsModel.getSelectedSample() &&
        *usedKick == kickModel.getSelectedSample() &&
        *usedSnare == snareModel.getSelectedSample()) {
      juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::NoIcon, "Nice",
                                             "Nice", "OK", this,
                                             new ModalReturn{*this});
    } else {
      juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::NoIcon, "Bad",
                                             "Wrongs Samples", "OK", this,
                                             new ModalReturn{*this});
    }
  }
}

void QuizzComponent::nextQuestion() {
  samplerSource.clearSamples();
  kickList.deselectAllRows();
  snareList.deselectAllRows();
  hatsList.deselectAllRows();

  auto randomSample = [](SampleListModel &model) -> SampleInfos & {
    auto id = juce::Random().nextInt(model.getNumRows());
    return model.getSampleAt(id);
  };

  usedKick = &randomSample(kickModel);
  usedSnare = &randomSample(snareModel);
  usedHats = &randomSample(hatsModel);

  samplerSource.addSample(*usedKick, cache);
  samplerSource.addSample(*usedSnare, cache);
  samplerSource.addSample(*usedHats, cache);

  play();
}

void QuizzComponent::endGame() { stop(); }

void QuizzComponent::setupMidi() {
  auto list = juce::MidiInput::getAvailableDevices();

  for (auto &input : list) {
    if (!deviceManager.isMidiInputDeviceEnabled(input.identifier))
      deviceManager.setMidiInputDeviceEnabled(input.identifier, true);
    deviceManager.addMidiInputDeviceCallback(input.identifier,
                                             &samplerSource.getMidiCollector());
  }
}