
#include <lager/lenses/unbox.hpp>
#include "juce_audio_utils/juce_audio_utils.h"

#include "../Samples/SampleInfos.h"
#include "Game.h"
#include "QuizzComponent.h"
#include <utility>

QuizzComponent::QuizzComponent(lager::reader<Quizz::model> model_,
                               lager::context<Quizz::quizzAction> ctx_)
    :
      game_model{std::move(model_)},
      step{game_model[&Quizz::model::type]},
      ctx(std::move(ctx_)),

      hats_list(
          game_model[&Quizz::model::hats][&Quizz::sampleContainer::samples]
                    [lager::lenses::unbox],
          game_model[&Quizz::model::hats]
                    [&Quizz::sampleContainer::selected_index],
          ctx),
      snare_list(
          game_model[&Quizz::model::snares][&Quizz::sampleContainer::samples]
          [lager::lenses::unbox],
          game_model[&Quizz::model::snares]
          [&Quizz::sampleContainer::selected_index],
          ctx),
      kick_list(
          game_model[&Quizz::model::kicks][&Quizz::sampleContainer::samples]
          [lager::lenses::unbox],
          game_model[&Quizz::model::kicks]
          [&Quizz::sampleContainer::selected_index],
          ctx),

      current_score_label(
          "correct : ",
          game_model[&Quizz::model::score][&Quizz::Score::correct_anwsers]),
      total_score_label(
          "total : ",
          game_model[&Quizz::model::score][&Quizz::Score::total_answers]) {

    addAndMakeVisible(kick_list);
    addAndMakeVisible(snare_list);
    addAndMakeVisible(hats_list);

    addAndMakeVisible(startButton);
    addChildComponent(answerButton);
    addAndMakeVisible(backButton);

    addChildComponent(current_score_label);
    addChildComponent(total_score_label);

    startButton.onClick = [this] { startClicked(); };
    answerButton.onClick = [this] { answerClicked(); };
    backButton.onClick = [this] { backPressed(); };

    watch(step, [this](Quizz::StepType newType) { typeChanged(newType); });
}

void QuizzComponent::resized()
{
  auto bounds = getLocalBounds().reduced(10);

  int bottomHeight = 40;
  int topHeight = 30;

  auto listWidth = bounds.getWidth() / 3;

  auto topBound = bounds.withHeight(topHeight);
  current_score_label.setBounds(topBound.withTrimmedLeft(topBound.getWidth()/2));
  total_score_label.setBounds(topBound.withTrimmedRight(topBound.getWidth()/2));

  auto listBound = bounds
                       .withTrimmedBottom(bottomHeight)
                       .withTrimmedTop(topHeight)
                       .withWidth(listWidth);

  kick_list.setBounds(listBound.reduced(3));
  snare_list.setBounds((listBound + juce::Point(listWidth, 0)).reduced(3));
  hats_list.setBounds((listBound + juce::Point(listWidth * 2, 0)).reduced(3));


  auto buttonRow = bounds.withTop(listBound.getBottom()).reduced(3);
  auto startAndAnswerBound = buttonRow.withLeft(buttonRow.getWidth() - 120);
  answerButton.setBounds(startAndAnswerBound);
  startButton.setBounds(startAndAnswerBound);
  auto backBound = startAndAnswerBound - juce::Point<int>{140 , 0};
  backButton.setBounds(backBound);
}
