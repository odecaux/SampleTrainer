#pragma once
#include <algorithm>
#include <memory>
#include <utility>

#include <juce_core/juce_core.h>

#include "Samples/SampleInfos.h"
#include "Samples/SampleBufferCache.h"
#include "Samples/SampleRepository.h"
#include "Samples/SamplePlayer.h"

#include "SampleSelector/SampleSelectorPanel.h"

#include "Quizz/SampleVoice.h"
#include "Quizz/Sampler.h"
#include "Quizz/Sequencer.h"
#include "Quizz/QuizzSource.h"
#include "Quizz/QuizzComponent.h"

using namespace juce;

class MainComponent : public Component {
public:
  //==============================================================================
  MainComponent() {
    mFormatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2);

    cache = std::make_unique<SampleBufferCache>(mFormatManager);

    sampleSelector = std::make_unique<SampleSelectorPanel>(deviceManager, *cache, repository);
    quizz = std::make_unique<QuizzComponent>(deviceManager, *cache);

    sampleSelector->setOnStartButtonClick(
        [&](std::vector<SampleInfos> &&samples)
        {
          showComponent(quizz.get());
          quizz->addSamples(std::move(samples));
        });

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
  AudioFormatManager mFormatManager;
  juce::AudioDeviceManager deviceManager;

  std::unique_ptr<SampleSelectorPanel> sampleSelector;
  std::unique_ptr<QuizzComponent> quizz;

  juce::WeakReference<Component> panelComponent;

  SampleRepository repository;
  std::unique_ptr<SampleBufferCache> cache;

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
