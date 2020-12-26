//
// Created by Octave on 18/12/2020.
//
#include <lager/cursor.hpp>
#include <lager/watch.hpp>
#include <lager/context.hpp>

class QuizzSampleListComponent : public juce::Component,
                                 public juce::ListBoxModel
{
  lager::reader<Quizz::sampleContainer> model_;
  lager::context<Quizz::selectSample> ctx_;
  juce::ListBox listBox;

public:
  QuizzSampleListComponent(lager::reader<Quizz::sampleContainer> model,
                           lager::context<Quizz::selectSample> ctx)
      : model_(std::move(model)),
        ctx_(std::move(ctx))
  {
    model_.watch([this](auto&&...){
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

  int getNumRows() override {return model_->size();}

  void listBoxItemClicked(int row, const juce::MouseEvent &mouseEvent) override
  {
    ctx_.dispatch(Quizz::selectSample{ model_->samples.get()[row].type, row });
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
    DBG(model_->selected_index.value_or(-1));
    if (model_->selected_index == rowNumber)
      g.fillAll(juce::Colours::lightblue);
    else if (rowNumber % 2)
      g.fillAll(alternateColour);

    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    g.setFont(juce::Font{});

    if (rowNumber < getNumRows()) {
      auto cellText = model_->samples.get()[rowNumber].file.getFileName();
      g.drawText(cellText, 2, 0, width - 4, height,
                 juce::Justification::centredLeft, true);
    }
  }
};