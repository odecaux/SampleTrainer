//
// Created by Octave on 04/01/2021.
//

#ifndef SAMPLE_TRAINER_PANELS_H
#define SAMPLE_TRAINER_PANELS_H

#include <lager/state.hpp>
#include <lager/sensor.hpp>
#include <utility>
class AuditioningPanel : public juce::Component
{

  lager::reader<Quizz::Auditioning> state_;
  QuizzSampleListComponent kick_list;
  QuizzSampleListComponent snare_list;
  QuizzSampleListComponent hats_list;

  lager::context<Quizz::nextQuestion> ctx_;
  juce::TextButton startButton{"Start"};

  void answerClicked(){
    ctx_.dispatch(Quizz::nextQuestion{});
  }

public:
  AuditioningPanel(lager::reader<Quizz::Auditioning> state,
                   const lager::context<Quizz::quizzAction>& ctx,
                   const std::vector<SampleInfos>& kicks,
                   const std::vector<SampleInfos>& snare,
                   const std::vector<SampleInfos>& hats) :
  ctx_(ctx),
  state_(std::move(state)),
  kick_list(kicks, state_[&Quizz::Auditioning::kick_index][lager::lenses::force_opt], ctx, SampleType::kick),
  snare_list(snare, state_[&Quizz::Auditioning::snare_index][lager::lenses::force_opt], ctx, SampleType::snare),
  hats_list(hats, state_[&Quizz::Auditioning::hats_index][lager::lenses::force_opt], ctx, SampleType::hats)
  {
    addAndMakeVisible(kick_list);
    addAndMakeVisible(snare_list);
    addAndMakeVisible(hats_list);

    addAndMakeVisible(startButton);

    startButton.onClick = [this] { answerClicked(); };

  }


  void resized() override{
    auto bounds = getLocalBounds();
    int bottomHeight = 40;

    auto listWidth = bounds.getWidth() / 3;

    auto listBound = bounds
        .withTrimmedBottom(bottomHeight)
        .withWidth(listWidth);

    kick_list.setBounds(listBound.reduced(3));
    snare_list.setBounds((listBound + juce::Point(listWidth, 0)).reduced(3));
    hats_list.setBounds((listBound + juce::Point(listWidth * 2, 0)).reduced(3));


    auto buttonRow = bounds.withTop(listBound.getBottom()).reduced(3);
    auto startBounds = buttonRow.withLeft(buttonRow.getWidth() - 120);
    startButton.setBounds(startBounds);
  }

  void paint(juce::Graphics &g) override {
  }
};

class QuestionPanel : public juce::Component
{

  lager::reader<Quizz::Question> state_;
  QuizzSampleListComponent kick_list;
  QuizzSampleListComponent snare_list;
  QuizzSampleListComponent hats_list;

  lager::context<Quizz::answerQuestion> ctx_;
  juce::TextButton answerButton{"Answer"};

  void answerClicked(){ ctx_.dispatch(Quizz::answerQuestion{}); }

public:
  QuestionPanel(lager::reader<Quizz::Question> state,
                   const lager::context<Quizz::quizzAction>& ctx,
                   const std::vector<SampleInfos>& kicks,
                   const std::vector<SampleInfos>& snare,
                   const std::vector<SampleInfos>& hats) :
  ctx_(ctx),
  state_(std::move(state)),
  kick_list(kicks, state_[&Quizz::Question::answer_kick_index], ctx, SampleType::kick),
  snare_list(snare, state_[&Quizz::Question::answer_snare_index], ctx, SampleType::snare),
  hats_list(hats, state_[&Quizz::Question::answer_hats_index], ctx, SampleType::hats)
  {
    addAndMakeVisible(kick_list);
    addAndMakeVisible(snare_list);
    addAndMakeVisible(hats_list);

    answerButton.onClick = [this] { answerClicked(); };

    addAndMakeVisible(answerButton);
  }


  void resized() override{
    auto bounds = getLocalBounds();
    int bottomHeight = 40;

    auto listWidth = bounds.getWidth() / 3;

    auto listBound = bounds
        .withTrimmedBottom(bottomHeight)
        .withWidth(listWidth);

    kick_list.setBounds(listBound.reduced(3));
    snare_list.setBounds((listBound + juce::Point(listWidth, 0)).reduced(3));
    hats_list.setBounds((listBound + juce::Point(listWidth * 2, 0)).reduced(3));


    auto buttonRow = bounds.withTop(listBound.getBottom()).reduced(3);
    auto startBounds = buttonRow.withLeft(buttonRow.getWidth() - 120);
    answerButton.setBounds(startBounds);
  }

  void paint(juce::Graphics &g) override {
  }
};


class PausePanel : public juce::Component, private juce::Timer
{

  juce::Label was_right_label;
  juce::TextButton nextButton{"Next Question"};
  const lager::context<Quizz::quizzAction>& ctx_;

  void toNextQuestion(){
    ctx_.dispatch(Quizz::nextQuestion{});
  }

public:
  explicit PausePanel(const lager::context<Quizz::quizzAction>& ctx, bool was_right)
      : ctx_(ctx)
  {
    nextButton.onClick = [this] { toNextQuestion(); };
    addAndMakeVisible(nextButton);

    was_right_label.setText(was_right ? "Right answer !" : "Wrong answer !", juce::dontSendNotification);
    was_right_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(was_right_label);

    startTimer(2000);
  }

  void resized() override{
    auto centre = getLocalBounds().getCentre();
    auto buttonCentre = centre + juce::Point<int>{0,50};
    auto labelCentre = centre + juce::Point<int>{0,-50};

    nextButton.setBounds(juce::Rectangle{150,40}
                             .withCentre(buttonCentre));
    was_right_label.setBounds(juce::Rectangle{150,40}
                             .withCentre(labelCentre));
  }

  void paint(juce::Graphics &g) override {
  }

  void timerCallback() override{
    stopTimer();
    toNextQuestion();
  }
};



#endif // SAMPLE_TRAINER_PANELS_H
