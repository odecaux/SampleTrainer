#pragma once

class SampleSelectorComponent : public juce::Component,
                                public juce::TableListBoxModel,
                                public juce::FileDragAndDropTarget

{
public:
  SampleSelectorComponent(juce::AudioDeviceManager &dm, SampleBufferCache &sl);

  void resized() override;

  //------------------------------------------------------

  int getNumRows() override { return data.getNumRows(); }

  void paintRowBackground(juce::Graphics &g, int rowNumber, int /*width*/,
                          int /*height*/, bool rowIsSelected) override;

  void paintCell(juce::Graphics &g, int rowNumber, int columnId, int width,
                 int height, bool /*rowIsSelected*/) override;

  void sortOrderChanged(int newSortColumnId, bool isForwards) override;

  Component *
  refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
                          Component *existingComponentToUpdate) override;

  int getColumnAutoSizeWidth(int columnId) override;

  void cellClicked(int rowNumber, int columnId,
                   const juce::MouseEvent &) override;

  void deleteKeyPressed(int) override;

  //-------------------------------------------------------

  void filesDropped(const juce::StringArray &files, int /*x*/,
                    int /*y*/) override;

  bool isInterestedInFileDrag(const juce::StringArray &files) override {
    return true;
  }

  //-------------------------------------------------------

  void setOnStartButtonClick(
      const std::function<void(std::vector<SampleInfos> &&)> &action);

  void saveRepositoryToDisk();

  void loadRepositoryFromDisk();

private:
  juce::TableListBox table; // the table component itself
  juce::TextButton startButton{"Start"};
  juce::Font font{14.0f};
  SampleRepository data;
  SampleSelectorAudio audio;

  void setSampleType(int row, SampleType type) {
    data.setSampleType(row, type);
  }

  SampleType getSampleType(int row) { return data.getSampleType(row); }

  //==============================================================================
  // This is a custom component containing a combo box, which we're going to put
  // inside our table's "rating" column.
  class SampleTypeCustomComponent : public Component {
  public:
    explicit SampleTypeCustomComponent(SampleSelectorComponent &td)
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
    SampleSelectorComponent &owner;
    juce::ComboBox comboBox;
    int row{};
  };

  static void showMissingSamplesDialog() {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::NoIcon, "You baddie",
        "You need all three sample types", "OK");
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleSelectorComponent)
};