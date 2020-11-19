//
// Created by Octave on 18/11/2020.
//
#include <juce_core/juce_core.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "../Samples/SampleInfos.h"
#include "../Samples/SampleRepository.h"
#include "../Samples/SampleBufferCache.h"
#include "SampleSelectorAudio.h"
#include "SampleSelectorComponent.h"

SampleSelectorComponent::SampleSelectorComponent(juce::AudioDeviceManager &dm,
                                                 SampleBufferCache &sl)
    : audio(dm, sl) {

  addAndMakeVisible(table);
  table.setModel(this);
  table.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
  table.setOutlineThickness(1);
  table.setMultipleSelectionEnabled(true);
  table.setClickingTogglesRowSelection(true);

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

void SampleSelectorComponent::resized()
{
  auto bounds = getLocalBounds();
  bounds.reduce(8, 8);
  auto tableBounds = bounds.withTrimmedBottom(50);
  table.setBounds(tableBounds);
  auto startButtonBounds =
      bounds.withTrimmedTop(tableBounds.getHeight()).reduced(0, 8);
  startButton.setBounds(
      startButtonBounds.withLeft(startButtonBounds.getWidth() - 100));
}

void SampleSelectorComponent::paintRowBackground(juce::Graphics &g,
                                                 int rowNumber, int /*width*/,
                                                 int /*height*/,
                                                 bool rowIsSelected) {
  auto alternateColour =
      getLookAndFeel()
          .findColour(juce::ListBox::backgroundColourId)
          .interpolatedWith(
              getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
  if (rowIsSelected)
    g.fillAll(juce::Colours::lightblue);
  else if (rowNumber % 2)
    g.fillAll(alternateColour);
}

void SampleSelectorComponent::paintCell(juce::Graphics &g, int rowNumber,
                                        int columnId, int width, int height,
                                        bool /*rowIsSelected*/) {
  g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
  g.setFont(font);

  if (rowNumber < getNumRows()) {
    auto cellText = data.getFieldAsString(
        SampleRepository::idToColumn(columnId), rowNumber);
    g.drawText(cellText, 2, 0, width - 4, height,
               juce::Justification::centredLeft, true);
  }

  g.setColour(getLookAndFeel().findColour(juce::ListBox::backgroundColourId));
  g.fillRect(width - 1, 0, 1, height);
}

juce::Component* SampleSelectorComponent::refreshComponentForCell(
    int rowNumber, int columnId, bool /*isRowSelected*/,
    Component *existingComponentToUpdate) {
  if (columnId == 4) {
    auto *ratingsBox =
        dynamic_cast<SampleTypeCustomComponent *>(existingComponentToUpdate);

    if (ratingsBox == nullptr)
      ratingsBox = new SampleTypeCustomComponent(*this);

    ratingsBox->setRowAndColumn(rowNumber, columnId);
    return ratingsBox;
  } else // The ID and Length columns do not have a custom component
  {
    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
  }
}

void SampleSelectorComponent::sortOrderChanged(int newSortColumnId,
                                               bool isForwards) {
  if (newSortColumnId != 0) {
    data.sortByColumnId(newSortColumnId);
    table.updateContent();
  }
}
int SampleSelectorComponent::getColumnAutoSizeWidth(int columnId) {
  int widest = 100;

  for (int i = getNumRows(); --i >= 0;) {
    auto text =
        data.getFieldAsString(SampleRepository::idToColumn(columnId), i);
    widest = juce::jmax(widest, font.getStringWidth(text));
  }

  return widest + 8;
}

void SampleSelectorComponent::cellClicked(int rowNumber, int columnId,
                                          const juce::MouseEvent &)  {
  if (table.getSelectedRows().contains(rowNumber)) {
    auto &row = data.getSampleInfos(rowNumber);
    audio.playSound(row);
  } else
    audio.stop();
}

void SampleSelectorComponent::deleteKeyPressed(int)  {
  audio.stopAndRelease();
  data.removeSamples(table.getSelectedRows());
  table.updateContent();
}

void SampleSelectorComponent::filesDropped(const juce::StringArray &files,
                                           int /*x*/, int /*y*/)  {
  for (const auto &file : files) {
    data.createSample(file);
  }
  table.updateContent();
}

void SampleSelectorComponent::setOnStartButtonClick(
    const std::function<void(std::vector<SampleInfos> &&)> &action) {
  startButton.onClick = [action, this] {
    auto rowIds = table.getSelectedRows();
    if (rowIds.size() == 0)
      return;

    // TODO c'est super nul comme implémentation
    bool hasKick = false;
    bool hasSnare = false;
    bool hasHats = false;

    audio.stopAndRelease();

    std::vector<SampleInfos> samples;
    for (auto i = 0; i < rowIds.size(); ++i) {
      auto rowId = rowIds[i];
      auto &sampleInfos = data.getSampleInfos(rowId);

      switch (sampleInfos.type) {
      case SampleType::kick:
        hasKick = true;
        break;
      case SampleType::snare:
        hasSnare = true;
        break;
      case SampleType::hats:
        hasHats = true;
        break;
      }

      samples.push_back(sampleInfos);
    }
    if (hasKick && hasSnare && hasHats)
      action(std::move(samples));
    else
      showMissingSamplesDialog();
  };
}

void SampleSelectorComponent::saveRepositoryToDisk() {
  auto out = data.serialize();
  auto file =
      juce::File::getCurrentWorkingDirectory().getChildFile("table.csv");
  if (!file.existsAsFile()) {
    if (!file.create()) {
      juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::NoIcon, "Error",
                                             "Couldn't save table", "OK");
      return;
    }
  } else {
    file.replaceWithText(out);
  }
}

void SampleSelectorComponent::loadRepositoryFromDisk() {
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
      data.addSample(std::move(*sample));
  }
  table.updateContent();
}