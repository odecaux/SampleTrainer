#pragma once

#include <utility>

#include "QuizzSampleListComponent.h"
#include <lager/store.hpp>

class ScoreLabel : public juce::Component{
public:
  ScoreLabel(juce::String prefix_, lager::reader<int> score_ )
  : prefix(std::move(prefix_)),
        score(std::move(score_))
  {
    score.watch([this](int newValue){
      label.setText(prefix + juce::String{newValue}, juce::dontSendNotification);
    });
    label.setFont(juce::Font{});
    label.setText(prefix + juce::String{score.get()}, juce::dontSendNotification);
    addAndMakeVisible(label);
  }
private:
  juce::String prefix;
  lager::reader<int> score;
  juce::Label label;

  void resized() override
  {
    label.setBounds(getLocalBounds());
  }
};


class QuizzComponent final : public juce::Component {

  lager::reader<Quizz::model> game_model;
  lager::reader<Quizz::StepType> step;
  lager::context<Quizz::quizzAction> ctx;

  QuizzSampleListComponent hats_list;
  QuizzSampleListComponent snare_list;
  QuizzSampleListComponent kick_list;

  ScoreLabel total_score_label;
  ScoreLabel current_score_label;

  juce::TextButton startButton{"Start"};
  juce::TextButton answerButton{"Answer"};
  juce::TextButton backButton{"Back"};


public:
  explicit QuizzComponent(lager::reader<Quizz::model> model_,
                          lager::context<Quizz::quizzAction> ctx_);

  ~QuizzComponent() override = default;

  //-------------------------------------------------------------------------------

  void paint(juce::Graphics &g) override {}

  void resized() override;

private:

  void startClicked(){
    startNextQuestion();
  }

  void startNextQuestion(){
    ctx.dispatch(Quizz::nextQuestion{});
  }

  void answerClicked(){
    if (game_model->kicks.selected_index != std::nullopt &&
        game_model->snares.selected_index != std::nullopt &&
        game_model->hats.selected_index != std::nullopt)
      ctx.dispatch(
          Quizz::answerQuestion{game_model->kicks.selected_index.value(),
                                game_model->snares.selected_index.value(),
                                game_model->hats.selected_index.value()});
  }

  void backPressed(){
    ctx.dispatch(Quizz::leaveQuizz{});
  }

  void typeChanged(Quizz::StepType newType) {
    std::visit(lager::visitor{
                   [this](Quizz::Auditioning){
                     current_score_label.setVisible(false);
                     total_score_label.setVisible(false);
                     startButton.setVisible(true);
                     answerButton.setVisible(false);
                   },
                   [this](Quizz::Question){
                     current_score_label.setVisible(true);
                     total_score_label.setVisible(true);
                     startButton.setVisible(false);
                     answerButton.setVisible(true);
                   },
                   [this](Quizz::Pause){
                     juce::AlertWindow::showMessageBoxAsync(
                         juce::AlertWindow::NoIcon,
                         "wsh", "test",
                         "next", this,
                         juce::ModalCallbackFunction::create([this](int){startNextQuestion();}));
                   },
                   [this](Quizz::DisplayResults){
                     current_score_label.setVisible(true);
                     total_score_label.setVisible(true);
                     startButton.setVisible(false);
                     answerButton.setVisible(false);
                   },

    }, newType);

    if (std::holds_alternative<Quizz::Question>(newType))
      answerButton.setVisible(true);
    else
      answerButton.setVisible(false);
  }

};
