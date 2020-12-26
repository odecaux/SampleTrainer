
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

      hats_list(game_model[&Quizz::model::hats], ctx),
      snare_list(game_model[&Quizz::model::snares], ctx),
      kick_list(game_model[&Quizz::model::kicks], ctx),

      current_score_label("correct : ",
                          game_model[&Quizz::model::score][&Quizz::Score::correct_anwsers]),
      total_score_label("total : ",
                        game_model[&Quizz::model::score][&Quizz::Score::total_answers])
{

    addAndMakeVisible(kick_list);
    addAndMakeVisible(snare_list);
    addAndMakeVisible(hats_list);

    addAndMakeVisible(answerButton);
    addChildComponent(backButton);

    addAndMakeVisible(current_score_label);
    addAndMakeVisible(total_score_label);

    answerButton.onClick = [this] { answerClicked(); };

    watch(step,[this](Quizz::StepType newType){typeChanged(newType);});


    //TODO implementer panel, ecoute
    ctx.dispatch(Quizz::nextQuestion{});
}

void QuizzComponent::resized()
{
  auto bounds = getLocalBounds().reduced(10);

  int bottomHeight = 40;

  auto listWidth = bounds.getWidth() / 3;
  auto listBound = bounds.withTrimmedBottom(bottomHeight).withWidth(listWidth);

  kick_list.setBounds(listBound.reduced(3));
  snare_list.setBounds((listBound + juce::Point(listWidth, 0)).reduced(3));
  hats_list.setBounds((listBound + juce::Point(listWidth * 2, 0)).reduced(3));


  auto buttonRow = bounds.withTrimmedTop(listBound.getHeight()).reduced(3);
  answerButton.setBounds(buttonRow.withTrimmedLeft(buttonRow.getWidth() - 120));
}
