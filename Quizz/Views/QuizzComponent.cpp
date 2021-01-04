
#include <lager/lenses/unbox.hpp>
#include <lager/lenses/variant.hpp>
#include "juce_audio_utils/juce_audio_utils.h"

#include "../../Samples/SampleInfos.h"
#include "../Game.h"
#include "QuizzComponent.h"
#include <utility>

QuizzComponent::QuizzComponent(lager::reader<Quizz::model> model_,
                               lager::context<Quizz::quizzAction> ctx_)
    :
      game_model{std::move(model_)},
      step{game_model[&Quizz::model::type]},
      step_index(step.xform(zug::map([](auto s){return s.index();}))),
      ctx(std::move(ctx_)),

      hats_list(game_model[&Quizz::model::hats][lager::lenses::unbox],
                game_model[&Quizz::model::type].xform(
                    zug::map([](auto &&t) -> std::optional<int> {
                      if (std::holds_alternative<Quizz::Auditioning>(t))
                        return std::get<Quizz::Auditioning>(t).hats_index;
                      else if (std::holds_alternative<Quizz::Question>(t))
                        return std::get<Quizz::Question>(t).answer_hats_index;
                      else
                        return std::nullopt;
                    })),
                ctx),
      snare_list(game_model[&Quizz::model::snares][lager::lenses::unbox],
                 game_model[&Quizz::model::type].xform(
                     zug::map([](auto &&t) -> std::optional<int> {
                       if (std::holds_alternative<Quizz::Auditioning>(t))
                         return std::get<Quizz::Auditioning>(t).snare_index;
                       else if (std::holds_alternative<Quizz::Question>(t))
                         return std::get<Quizz::Question>(t).answer_snare_index;
                       else
                         return std::nullopt;
                     })),
                 ctx),
      kick_list(game_model[&Quizz::model::kicks][lager::lenses::unbox],
                game_model[&Quizz::model::type].xform(
                    zug::map([](auto &&t) -> std::optional<int> {
                      if (std::holds_alternative<Quizz::Auditioning>(t))
                        return std::get<Quizz::Auditioning>(t).kick_index;
                      else if (std::holds_alternative<Quizz::Question>(t))
                        return std::get<Quizz::Question>(t).answer_kick_index;
                      else
                        return std::nullopt;
                    })),
                ctx),

      current_score_label(
          "correct : ",
          game_model[&Quizz::model::score][&Quizz::Score::correct_anwsers]),
      total_score_label(
          "total : ",
          game_model[&Quizz::model::score][&Quizz::Score::total_answers]) {

  addAndMakeVisible(title);
  title.setJustificationType(juce::Justification::centred);
  title.setFont(juce::Font{15});

  addChildComponent(current_score_label);
  addChildComponent(total_score_label);
  addAndMakeVisible(kick_list);
  addAndMakeVisible(snare_list);
  addAndMakeVisible(hats_list);

  addAndMakeVisible(startButton);
  addChildComponent(answerButton);
  addAndMakeVisible(backButton);

  startButton.onClick = [this] { startClicked(); };
    answerButton.onClick = [this] { answerClicked(); };
    backButton.onClick = [this] { backPressed(); };

    watch(step, [this](Quizz::StepType newType) { typeChanged(newType); });

    auto title_update = [this](size_t index){
      //TODO dependency
      if(index == 0)
        title.setText("Listen to the samples", juce::dontSendNotification);
      else if(index == 1)
        title.setText("Identify the samples", juce::dontSendNotification);
      else if(index == 3)
        title.setText("", juce::dontSendNotification);
    };
    watch(step_index, title_update);
    title_update(*step_index);
}

void QuizzComponent::resized()
{
  auto totalBounds = getLocalBounds().reduced(10);

  auto titleHeight = 30;

  auto titleBounds = totalBounds.withHeight(titleHeight);
  title.setBounds(titleBounds);

  auto bounds = totalBounds.withTrimmedTop(titleHeight);

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
