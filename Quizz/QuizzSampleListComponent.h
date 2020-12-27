//
// Created by Octave on 18/12/2020.
//
#include <lager/cursor.hpp>
#include <utility>
#include <lager/watch.hpp>
#include <lager/context.hpp>

class QuizzSampleListComponent : public juce::Component,
                                 public juce::ListBoxModel
{
  lager::reader<std::vector<SampleInfos>> samples;
  lager::reader<std::optional<int>> index_;
  lager::context<Quizz::selectSample> ctx_;
  juce::ListBox listBox;

public:
  QuizzSampleListComponent(lager::reader<std::vector<SampleInfos>> samples,
                           lager::reader<std::optional<int>> index,
                           lager::context<Quizz::selectSample> ctx)
      : samples(std::move(samples)), index_(std::move(index)),
        ctx_(std::move(ctx))
  {
    watch(index_, [this](std::optional<int> new_index){
      listBox.updateContent();
      repaint();
    });


    listBox.setModel(this);
    addAndMakeVisible(listBox);
  }

private:
  void resized() override
  {
    listBox.setBounds(getLocalBounds());
  }

  int getNumRows() override {return samples->size();}

  void listBoxItemClicked(int row, const juce::MouseEvent &mouseEvent) override
  {
    auto prev = *index_;
    ctx_.dispatch(Quizz::selectSample{ samples.get()[row].type, row });
    std::cout<< (prev != *index_) <<std::endl;
  }

  void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height,
                        bool rowIsSelected) override
  {

    auto alternateColour =
        getLookAndFeel()
            .findColour(juce::ListBox::backgroundColourId)
            .interpolatedWith(
                getLookAndFeel().findColour(juce::ListBox::textColourId),
                0.03f);

    if (*index_ == rowNumber)
      g.fillAll(juce::Colours::lightblue);
    else if (rowNumber % 2)
      g.fillAll(alternateColour);

    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    g.setFont(juce::Font{});

    if (rowNumber < getNumRows()) {
      auto cellText = samples.get()[rowNumber].file.getFileName();
      g.drawText(cellText, 2, 0, width - 4, height,
                 juce::Justification::centredLeft, true);
    }
  }
};