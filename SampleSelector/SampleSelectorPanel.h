#pragma once
#include <optional>

class Table : public juce::TableListBoxModel,
              public juce::Component
{
public:

  Table(SamplePlayer & audio, SampleRepository& repo);

  void filesDropped(const juce::StringArray &fileNames);

  void deleteKeyPressed(int) override;

  SampleType getSampleType(int row) { return repository.getSampleType(row); }
  void setSampleType(int row, SampleType type) { repository.setSampleType(row, type); }

  auto getSelectedSamplesIfValid() -> std::optional<std::vector<SampleInfos>>
  {
    auto rowIds = getSelectedRows();
    if (rowIds.size() == 0)
      return std::nullopt;

    audio.stopAndRelease();

    auto samples = repository.getSampleInfos(rowIds);
    bool hasKick = std::ranges::any_of(
        samples, [](auto &sample) { return sample.type == kick; });
    bool hasSnare = std::ranges::any_of(
        samples, [](auto &sample) { return sample.type == snare; });
    bool hasHats = std::ranges::any_of(
        samples, [](auto &sample) { return sample.type == hats; });

    if (hasKick && hasSnare && hasHats)
      return samples;
    else
      return std::nullopt;
  }

  void resized() override { table.setBounds(getLocalBounds()); }

private:
  int getNumRows() override { return repository.getNumRows(); }

  void paintRowBackground(juce::Graphics &g, int rowNumber, int /*width*/,
                          int /*height*/, bool rowIsSelected) override;

  void paintCell(juce::Graphics &g, int rowNumber, int columnId, int width,
                 int height, bool /*rowIsSelected*/) override;

  void sortOrderChanged(int newSortColumnId, bool isForwards) override;

  juce::Component *
  refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
                          juce::Component *existingComponentToUpdate) override;

  int getColumnAutoSizeWidth(int columnId) override;

  void cellClicked(int rowNumber, int columnId,
                   const juce::MouseEvent &) override;

  auto getSelectedRows() -> juce::SparseSet<int> {
    return table.getSelectedRows();
  }


  juce::TableListBox table;
  SampleRepository& repository;
  SamplePlayer & audio;


  class SampleTypeCustomComponent : public Component {
  public:
    explicit SampleTypeCustomComponent(Table &td)
        : owner(td) {

      addAndMakeVisible(comboBox);
      comboBox.addItem("kick", SampleType::kick);
      comboBox.addItem("snare", SampleType::snare);
      comboBox.addItem("hats", SampleType::hats);

      comboBox.onChange = [this] {
        owner.setSampleType(row, SampleType(comboBox.getSelectedId()));
      };
      comboBox.setWantsKeyboardFocus(false);
    }

    void resized() override {
      comboBox.setBoundsInset(juce::BorderSize<int>(2));
    }

    void setRowAndColumn(int newRow, int) {
      row = newRow;
      comboBox.setSelectedId(owner.getSampleType(row),
                             juce::dontSendNotification);
    }

  private:
    Table &owner;
    juce::ComboBox comboBox;
    int row{};
  };
};


class SampleSelectorPanel : public juce::Component,
                            public juce::FileDragAndDropTarget

{
public:
  SampleSelectorPanel(SamplePlayer& player,
                      SampleRepository &sr);

  void resized() override;

  void filesDropped(const juce::StringArray &files, int /*x*/,
                    int /*y*/) override;

  bool isInterestedInFileDrag(const juce::StringArray &files) override {
    return true;
  }

  void setOnStartButtonClick(
      const std::function<void(std::vector<SampleInfos> &&)> &action);

private:
  juce::TextButton startButton{"Start"};
  juce::Font font{14.0f};

  SamplePlayer& audio;
  Table table;

  static void showMissingSamplesDialog() {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::NoIcon, "You badie",
        "You need all three sample types", "OK");
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleSelectorPanel)
};
