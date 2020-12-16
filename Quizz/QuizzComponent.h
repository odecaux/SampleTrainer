#pragma once

// TODO separate component, midi/audio and game logic

class QuizzComponent final : public juce::Component {
public:
  explicit QuizzComponent(juce::AudioDeviceManager &dm,
                          SampleBufferCache &cache);

  ~QuizzComponent() override;

  //-------------------------------------------------------------------------------

  void paint(juce::Graphics &g) override {}

  void resized() override;

  //-------------------------------------------------------------------------------

  void addSamples(std::vector<SampleInfos> &&samples);

  void play() { samplerSource.play(); }
  void stop() { samplerSource.stop(); }

private:
  class SampleListModel : public juce::ListBoxModel {
  public:
    explicit SampleListModel(juce::Component &parent) : parent(parent) {}

    SampleListModel() = delete;

    void addSample(SampleInfos &sample) {
      samples.push_back(std::move(sample));
    }
    int getNumRows() override { return samples.size(); }

    void listBoxItemClicked(int row, const juce::MouseEvent &mouseEvent) override {
      lastRow = row;
    }

    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height,
                          bool rowIsSelected) override {

      auto alternateColour =
          parent.getLookAndFeel()
              .findColour(juce::ListBox::backgroundColourId)
              .interpolatedWith(parent.getLookAndFeel().findColour(
                                    juce::ListBox::textColourId),
                                0.03f);
      if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
      else if (rowNumber % 2)
        g.fillAll(alternateColour);

      g.setColour(
          parent.getLookAndFeel().findColour(juce::ListBox::textColourId));
      g.setFont(font);

      if (rowNumber < getNumRows()) {
        auto cellText = samples[rowNumber].file.getFileName();
        g.drawText(cellText, 2, 0, width - 4, height,
                   juce::Justification::centredLeft, true);
      }
    }

    SampleInfos &getSelectedSample() {
      jassert(lastRow != -1);
      jassert(lastRow < samples.size());
      return samples[lastRow];
    }
    SampleInfos &getSampleAt(int rowId) {
      jassert(rowId >= 0 && rowId < samples.size());
      return samples[rowId];
    }

  private:
    int lastRow = -1;
    std::vector<SampleInfos> samples{};
    juce::Font font{14.0f};
    juce::Component &parent;
  };

  class ModalReturn : public juce::ModalComponentManager::Callback {
  public:
    explicit ModalReturn(QuizzComponent &parent) : parent(parent) {}
    QuizzComponent &parent;

    void modalStateFinished(int returnValue) override { parent.nextQuestion(); }
  };
  juce::AudioSourcePlayer audioSourcePlayer;
  QuizzSource samplerSource;
  juce::AudioDeviceManager &deviceManager;
  SampleBufferCache &cache;

  juce::ListBox kickList;
  juce::ListBox snareList;
  juce::ListBox hatsList;
  juce::TextButton answerButton{"answer"};

  SampleListModel kickModel;
  SampleListModel snareModel;
  SampleListModel hatsModel;

  SampleInfos *usedKick{};
  SampleInfos *usedSnare{};
  SampleInfos *usedHats{};

  void initGame();

  void checkAnswers();

  void nextQuestion();

  void endGame();

  // TODO bouger dans samplersource, avec dm en paramètre
  void setupMidi();
};
