#pragma once
#include <algorithm>
#include <memory>
#include <utility>

#include <lager/event_loop/manual.hpp>
#include <lager/reader.hpp>

#include <juce_core/juce_core.h>

#include "Samples/SampleInfos.h"
#include "Samples/SampleBufferCache.h"
#include "Samples/SampleRepository.h"
#include "Samples/SamplePlayer.h"

#include "SampleSelector/SampleSelectorPanel.h"

#include "Quizz/Game.h"
#include "Quizz/SampleVoice.h"
#include "Quizz/Sampler.h"
#include "Quizz/Sequencer.h"
#include "Quizz/QuizzSource.h"
#include "Quizz/QuizzComponent.h"

using namespace juce;

class MainComponent : public Component {
public:
  //==============================================================================
  MainComponent()
      : cache(formatManager),
        samplePlayer{deviceManager, cache}
  {
    formatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2);

    sampleSelector = std::make_unique<SampleSelectorPanel>(samplePlayer, repository);

    sampleSelector->setOnStartButtonClick(
      [&](std::vector<SampleInfos> &&samples) { showQuizz(std::move(samples)); });

    loadRepositoryFromDisk();
    showComponent(sampleSelector.get());

    setSize(600, 400);
  }

  ~MainComponent() override { saveRepositoryToDisk(); }

  //==============================================================================
  void paint(Graphics &g) override {}

  void resized() override {
    if (panelComponent)
      panelComponent->setBounds(getLocalBounds());
  }

private:
  AudioFormatManager formatManager;
  juce::AudioDeviceManager deviceManager;

  SampleRepository repository;
  SampleBufferCache cache;

  SamplePlayer samplePlayer;
  std::unique_ptr<SampleSelectorPanel> sampleSelector;

  std::unique_ptr<QuizzPlayer> quizzPlayer;
  std::unique_ptr<QuizzComponent> quizz;

  juce::WeakReference<Component> panelComponent;

  void showQuizz(std::vector<SampleInfos> &&samples)
  {
    auto store = lager::make_store<Quizz::quizzAction>(
        Quizz::new_model(samples),
        Quizz::update,
        lager::with_manual_event_loop{});

    quizzPlayer = std::make_unique<QuizzPlayer>(store, cache, deviceManager);
    quizz = std::make_unique<QuizzComponent>(store, store);

    showComponent(quizz.get());
  }

  void showComponent(juce::Component *newPanel) {
    if (newPanel != panelComponent) {
      if (panelComponent != nullptr) {
        panelComponent->setVisible(false);
        removeChildComponent(panelComponent);
      }
      panelComponent = newPanel;
      if (panelComponent != nullptr) {
        addChildComponent(panelComponent);
        newPanel->setVisible(true);
      }
      repaint();
    }
    resized();
  }


  void saveRepositoryToDisk() {
    auto out = repository.serialize();
    auto file =
        juce::File::getCurrentWorkingDirectory().getChildFile("table.csv");
    if (!file.existsAsFile()) {
      if (!file.create()) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::NoIcon, "Error",
                                               "Couldn't save table", "OK");
        return;
      }
    }
    else {
      file.replaceWithText(out);
    }
  }

  void loadRepositoryFromDisk() {
    auto file =
        juce::File::getCurrentWorkingDirectory().getChildFile("table.csv");

    if (!file.existsAsFile())
      return;
    juce::StringArray lines;
    file.readLines(lines);
    for (const auto &line : lines) {
      if (line.isEmpty())
        break;
      auto sample = SampleInfos::deserialize(line);
      if (sample)
        repository.addSample(std::move(*sample));
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
