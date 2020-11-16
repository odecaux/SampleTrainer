#pragma once

#include <memory>
#include <utility>

class SampleSelectorComponent : public juce::Component,
                                public juce::TableListBoxModel,
                                public juce::FileDragAndDropTarget,
                                public juce::KeyListener

{
    //TODO supprimer le keyListener
public:
    SampleSelectorComponent(juce::AudioDeviceManager& dm,
                            SampleBufferCache& sl)
    : audio(dm, sl)
    {

        addAndMakeVisible(table);
        table.setModel(this);
        table.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
        table.setOutlineThickness(1);
        table.setMultipleSelectionEnabled(true);
        table.setClickingTogglesRowSelection(true);

        table.setWantsKeyboardFocus(true);
        table.addKeyListener(this);

        auto &header = table.getHeader();

        header.addColumn("ID", 1, 100);
        header.addColumn("path", 2, 100);
        header.addColumn("name", 3, 100);
        header.addColumn("sample type", 4, 100);
        header.addColumn("rank", 5, 100);

        // we could now change some initial settings..
        header.setSortColumnId(1, true);
        header.setColumnVisible(2, false);

        addAndMakeVisible(startButton);
    }

    int getNumRows() override
    {
        return data.getNumRows();
    }

    void
    paintRowBackground(juce::Graphics &g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
                .interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll(juce::Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll(alternateColour);
    }

    void paintCell(juce::Graphics &g, int rowNumber, int columnId,
                   int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
        g.setFont(font);

        if (rowNumber < getNumRows()) {
            auto cellText = data.getFieldAsString(SampleRepository::idToColumn(columnId), rowNumber);
            g.drawText(cellText, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        }

        g.setColour(getLookAndFeel().findColour(juce::ListBox::backgroundColourId));
        g.fillRect(width - 1, 0, 1, height);
    }

    void sortOrderChanged(int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0) {
            data.sortByColumnId(newSortColumnId);
            table.updateContent();
        }
    }

    Component *refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
                                       Component *existingComponentToUpdate) override
    {
        if (columnId == 4)
        {
            auto *ratingsBox = dynamic_cast<SampleTypeCustomComponent*>(existingComponentToUpdate);

            if (ratingsBox == nullptr)
                ratingsBox = new SampleTypeCustomComponent(*this);

            ratingsBox->setRowAndColumn(rowNumber, columnId);
            return ratingsBox;
        } else // The ID and Length columns do not have a custom component
        {
            jassert (existingComponentToUpdate == nullptr);
            return nullptr;
        }
    }

    int getColumnAutoSizeWidth(int columnId) override
    {
        int widest = 100;

        for (int i = getNumRows(); --i >= 0;) {
            auto text = data.getFieldAsString(SampleRepository::idToColumn(columnId), i);
            widest = juce::jmax(widest, font.getStringWidth(text));
        }

        return widest + 8;
    }


    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.reduce(8,8);
        auto tableBounds = bounds.withTrimmedBottom(50);
        table.setBounds( tableBounds );
        auto startButtonBounds = bounds.withTrimmedTop(tableBounds.getHeight())
                .reduced(0,8);
        startButton.setBounds(startButtonBounds.withLeft(startButtonBounds.getWidth() - 100));
    }

    void filesDropped(const juce::StringArray &files, int /*x*/, int /*y*/) override
    {
        for (const auto &file : files) {
            data.createSample(file);
        }
        table.updateContent();
    }

    bool isInterestedInFileDrag(const juce::StringArray &files) override
    {
        return true;
    }

    void setOnStartButtonClick(const std::function<void(std::vector<SampleInfos>&&)>& action)
    {
        startButton.onClick = [action, this] {
            auto rowIds = table.getSelectedRows();
            if(rowIds.size() == 0)
                return;

            //TODO c'est super nul comme implémentation
            bool hasKick = false;
            bool hasSnare = false;
            bool hasHats = false;

            audio.stopAndRelease();

            std::vector<SampleInfos> samples;
            for (auto i = 0; i < rowIds.size(); ++i) {
                auto rowId = rowIds[i];
                auto& sampleInfos = data.getSampleInfos(rowId);

                switch (sampleInfos.type) {
                    case kick:
                        hasKick = true;
                        break;
                    case snare:
                        hasSnare = true;
                        break;
                    case hats:
                        hasHats = true;
                        break;
                }

                //TODO copy/move constructor pour SampleInfos
                samples.push_back(sampleInfos);
            }
            if(hasKick && hasSnare && hasHats)
                action(std::move(samples));
            else
                showMissingSamplesDialog();
        };
    }

    bool keyPressed(const juce::KeyPress& key,
                    juce::Component* originatingComponent) override
    {
        if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
        {
            audio.stopAndRelease();
            data.removeSamples(table.getSelectedRows());
            table.updateContent();
            return true;
        }
        return false;
    }
    void selectedRowsChanged(int lastRowSelected) override
    {
        if(lastRowSelected != -1){
            auto& row = data.getSampleInfos(lastRowSelected);
            audio.playSound(row);
        }
        else
            audio.stop();
    }

    void saveRepositoryToDisk()
    {
        auto out = data.serialize();
        auto file = juce::File::getCurrentWorkingDirectory().getChildFile("table.csv");
        if(!file.existsAsFile())
        {
            if(!file.create()){
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::NoIcon, "Error",
                                                        "Couldn't save table",
                                                        "OK");
                return;
            }
        }
        else {
            file.replaceWithText(out);
        }
    }

    void loadRepositoryFromDisk()
    {
        auto file = juce::File::getCurrentWorkingDirectory().getChildFile("table.csv");

        if(!file.existsAsFile())
            return;
        juce::StringArray lines;
        file.readLines(lines);
        for(const auto& line : lines)
        {
            if(line.isEmpty())
                break;
            auto sample = SampleInfos::deserialize(line);
            if(sample)
                data.addSample(std::move(*sample));
        }
        table.updateContent();
    }

private:
    juce::TableListBox table;     // the table component itself
    juce::TextButton startButton{"Start"};
    juce::Font font{14.0f};
    SampleRepository data;
    SampleSelectorAudio audio;



    void setSampleType(int row, SampleType type)
    {
        data.setSampleType(row, type);
    }

    SampleType getSampleType(int row )
    {
        return data.getSampleType(row);
    }

//==============================================================================
    // This is a custom component containing a combo box, which we're going to put inside
    // our table's "rating" column.
    class SampleTypeCustomComponent : public Component
    {
    public:
        explicit SampleTypeCustomComponent(SampleSelectorComponent &td) : owner(td)
        {
            // just put a combo box inside this component
            addAndMakeVisible(comboBox);
            comboBox.addItem("kick", kick);
            comboBox.addItem("snare", snare);
            comboBox.addItem("hats", hats);

            comboBox.onChange = [this] {
                owner.setSampleType(row, SampleType(comboBox.getSelectedId()));
            };
            comboBox.setWantsKeyboardFocus(false);
        }

        void resized() override
        {
            comboBox.setBoundsInset(juce::BorderSize<int>(2));
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn(int newRow, int newColumn)
        {
            row = newRow;
            comboBox.setSelectedId(owner.getSampleType(row), juce::dontSendNotification);
        }

    private:
        SampleSelectorComponent &owner;
        juce::ComboBox comboBox;
        int row{};
    };


    static void showMissingSamplesDialog()
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::NoIcon, "You baddie",
                                          "You need all three sample types",
                                          "OK");
    }




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleSelectorComponent)
};