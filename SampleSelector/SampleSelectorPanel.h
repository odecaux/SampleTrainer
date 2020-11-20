#pragma once
#include <optional>


//TODO ajouter des jassert
class Model : public juce::TableListBoxModel
{
public:

  explicit Model(SampleSelectorAudio& audio, SampleRepository& repo)
      : audio(audio), repository(repo) {}

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
  void deleteKeyPressed(int) override;

  SampleType getSampleType(int row) { return repository.getSampleType(row); }
  void setSampleType(int row, SampleType type) { repository.setSampleType(row, type); }

  std::function<void()> onContentChanged;

  void setTable(juce::TableListBox *newTable) { table = newTable; }

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

  void filesDropped(const juce::StringArray &fileNames);

private:
  juce::TableListBox* table = nullptr;
  SampleRepository& repository;
  SampleSelectorAudio& audio;

  auto getSelectedRows() -> juce::SparseSet<int> { return table->getSelectedRows();}

  //TODO not an ideal solution either, because
  //1) I need to keep a reference to the table
  //2) the header definition is still in the selectorComponent, there's some really tight coupling

  //TODO at some point I need to find a way to completely abstract away the table
};


class SampleSelectorPanel : public juce::Component,
                            public juce::FileDragAndDropTarget

{
public:
  SampleSelectorPanel(juce::AudioDeviceManager &dm,
                      SampleBufferCache &sl,
                      SampleRepository &sr);

  void resized() override;

  //-------------------------------------------------------
  void filesDropped(const juce::StringArray &files, int /*x*/,
                    int /*y*/) override;

  bool isInterestedInFileDrag(const juce::StringArray &files) override {
    return true;
  }

  //-------------------------------------------------------
  void setOnStartButtonClick(
      const std::function<void(std::vector<SampleInfos> &&)> &action);

private:
  juce::TableListBox table; // the table component itself
  juce::TextButton startButton{"Start"};
  juce::Font font{14.0f};

  SampleSelectorAudio audio;
  Model model;

  friend class Model;
  //==============================================================================
  // This is a custom component containing a combo box, which we're going to put
  // inside our table's "rating" column.
  class SampleTypeCustomComponent : public Component {
  public:
    explicit SampleTypeCustomComponent(Model &td)
        : owner(td) {
      // just put a combo box inside this component
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

    // Our demo code will call this when we may need to update our contents
    void setRowAndColumn(int newRow, int newColumn) {
      row = newRow;
      comboBox.setSelectedId(owner.getSampleType(row),
                             juce::dontSendNotification);
    }

  private:
    Model &owner;
    juce::ComboBox comboBox;
    int row{};
  };

  static void showMissingSamplesDialog() {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::NoIcon, "You baddie",
        "You need all three sample types", "OK");
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleSelectorPanel)
};