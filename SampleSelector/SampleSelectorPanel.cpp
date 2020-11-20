//
// Created by Octave on 18/11/2020.
//
#include <juce_audio_utils/juce_audio_utils.h>
#include <chrono>

#include "../Samples/SampleInfos.h"
#include "../Samples/SampleBufferCache.h"
#include "../Samples/SampleRepository.h"
#include "../Samples/SamplePlayer.h"
#include "SampleSelectorPanel.h"

void Model::paintRowBackground(juce::Graphics &g, int rowNumber, int /*width*/,
                               int /*height*/, bool rowIsSelected) {
  auto alternateColour =
      juce::Desktop::getInstance().getDefaultLookAndFeel()
          .findColour(juce::ListBox::backgroundColourId)
          .interpolatedWith(
              juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
  if (rowIsSelected)
    g.fillAll(juce::Colours::lightblue);
  else if (rowNumber % 2)
    g.fillAll(alternateColour);
}

void Model::paintCell(juce::Graphics &g, int rowNumber, int columnId, int width,
                      int height, bool /*rowIsSelected*/) {
  g.setColour(juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ListBox::textColourId));
  g.setFont(juce::Font());

  if (rowNumber < getNumRows()) {
    auto cellText = repository.getFieldAsString(
        SampleRepository::idToColumn(columnId), rowNumber);
    g.drawText(cellText, 2, 0, width - 4, height,
               juce::Justification::centredLeft, true);
  }

  g.setColour(juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ListBox::backgroundColourId));
  g.fillRect(width - 1, 0, 1, height);
}

juce::Component *
Model::refreshComponentForCell(int rowNumber, int columnId,
                               bool /*isRowSelected*/,
                               juce::Component *existingComponentToUpdate) {
  if (columnId == 4) {
    auto *ratingsBox =
        dynamic_cast<SampleSelectorPanel::SampleTypeCustomComponent *>(
            existingComponentToUpdate);

    if (ratingsBox == nullptr)
      ratingsBox =
          new SampleSelectorPanel::SampleTypeCustomComponent(*this);

    ratingsBox->setRowAndColumn(rowNumber, columnId);
    return ratingsBox;
  } else // The ID and Length columns do not have a custom component
  {
    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
  }
}

void Model::sortOrderChanged(int newSortColumnId, bool isForwards) {
  if (newSortColumnId != 0) {
    repository.sortByColumnId(newSortColumnId);
    onContentChanged();
  }
}
int Model::getColumnAutoSizeWidth(int columnId) {
  int widest = 100;

  for (int i = getNumRows(); --i >= 0;) {
    auto text =
        repository.getFieldAsString(SampleRepository::idToColumn(columnId), i);
    widest = juce::jmax(widest, juce::Font().getStringWidth(text));
  }

  return widest + 8;
}

void Model::cellClicked(int rowNumber, int columnId, const juce::MouseEvent &) {

  if (getSelectedRows().contains(rowNumber)) {
    auto &row = repository.getSampleInfos(rowNumber);
    audio.playSound(row);
  } else
    audio.stop();

}

void Model::deleteKeyPressed(int) {
  audio.stopAndRelease();
  repository.removeSamples(getSelectedRows());
  onContentChanged();
}


void Model::filesDropped(const juce::StringArray &fileNames) {
  for(const auto& fileName : fileNames)
    repository.createSample(fileName);
}

SampleSelectorPanel::SampleSelectorPanel(juce::AudioDeviceManager &dm,
                                         SampleBufferCache &sl,
                                         SampleRepository &sr)
    : audio(dm, sl), model(audio, sr) {

  model.onContentChanged = [&]{ table.updateContent();};
  model.setTable(&table);

  table.setModel(&model);
  table.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
  table.setOutlineThickness(1);
  table.setMultipleSelectionEnabled(true);
  table.setClickingTogglesRowSelection(true);

  addAndMakeVisible(table);

  auto &header = table.getHeader();

  header.addColumn("ID", 1, 40, 30, 40);
  header.addColumn("path", 2, 100);
  header.addColumn("name", 3, 150, 100);
  header.addColumn("sample type", 4, 100, 75);
  header.addColumn("rank", 5, 30);

  // we could now change some initial settings..
  header.setSortColumnId(1, true);
  header.setColumnVisible(2, false);
  header.setColumnVisible(5, false);

  addAndMakeVisible(startButton);
}

void SampleSelectorPanel::resized() {
  auto bounds = getLocalBounds();
  bounds.reduce(8, 8);
  auto tableBounds = bounds.withTrimmedBottom(50);
  table.setBounds(tableBounds);
  auto startButtonBounds =
      bounds.withTrimmedTop(tableBounds.getHeight()).reduced(0, 8);
  startButton.setBounds(
      startButtonBounds.withLeft(startButtonBounds.getWidth() - 100));
}

void SampleSelectorPanel::filesDropped(const juce::StringArray &fileNames,
                                           int /*x*/, int /*y*/) {
  model.filesDropped(fileNames);
  table.updateContent();
}

void SampleSelectorPanel::setOnStartButtonClick(
    const std::function<void(std::vector<SampleInfos> &&)> &action) {

  startButton.onClick = [action, this] {
    if (auto samples = model.getSelectedSamplesIfValid())
      action(std::move(*samples));
    else
      showMissingSamplesDialog();
  };
}
