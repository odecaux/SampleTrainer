#pragma once

#include <utility>

#include "QuizzSampleListComponent.h"
#include "Panels.h"
#include <lager/store.hpp>
#include <lager/lenses/variant.hpp>
#include <zug/transducer/filter.hpp>



class ScoreLabel : public juce::Component{
public:
  ScoreLabel(juce::String prefix_, lager::reader<int> score_ )
  : prefix(std::move(prefix_)),
        score(std::move(score_))
  {
    score.watch([this](int newValue){
      draw(newValue);
    });
    draw(*score);
    addAndMakeVisible(label);
    label.setJustificationType(juce::Justification::centred);
  }
private:
  void draw(int value){
    label.setText(prefix + juce::String{score.get()}, juce::dontSendNotification);
  }
  juce::String prefix;
  lager::reader<int> score;
  juce::Label label;

  void resized() override
  {
    label.setBounds(getLocalBounds());
  }
};

class StepLabel : public juce::Component{
  lager::reader<Quizz::StepType> step_;
  juce::Label label;
public:
  explicit StepLabel(lager::reader<Quizz::StepType> step)
  : step_(std::move(step))
  {
    step_.watch([this](Quizz::StepType newValue){
      draw(newValue);
    });
    draw(*step_);
    addAndMakeVisible(label);
    label.setJustificationType(juce::Justification::centred);
  }
private:
  void draw(Quizz::StepType newStep){
    auto step_title = std::visit(lager::visitor{
      [](Quizz::Auditioning){ return "Listen to the samples";},
      [](Quizz::Question){ return "Guess which samples are used";},
      [](Quizz::Pause){ return "";},
      [](Quizz::DisplayResults){ return "End";},
    },newStep);

    label.setText(step_title, juce::dontSendNotification);
  }

  void resized() override { label.setBounds(getLocalBounds()); }
};



class QuizzComponent : public juce::Component {

  lager::reader<Quizz::model> game_model;
  lager::reader<size_t> step_index;
  lager::context<Quizz::quizzAction> ctx_;


  ScoreLabel total_score_label;
  ScoreLabel current_score_label;

  StepLabel step_title;

  std::unique_ptr<juce::Component> panel;

  template<typename T>
  struct alt {
    template <typename F>
    auto operator()(F &&reducer) const {
      return [=](auto &&s, auto &&is) mutable {
        return std::holds_alternative<T>(is)
                   ? reducer(s, std::get<T>(is))
                   : s;
      };
    }
  };


  void updatePanel(size_t new_step_index){
    switch(new_step_index){
      case 0:
        panel = std::make_unique<AuditioningPanel>(
            game_model[&Quizz::model::type]
                          .xform(alt<Quizz::Auditioning>{})
                          .make(),
                ctx_,
                game_model->kicks.get(),
                game_model->snares.get(),
                game_model->hats.get());
        break;

      case 1:
        panel = std::make_unique<QuestionPanel>(
            game_model[&Quizz::model::type]
                .xform(alt<Quizz::Question>{})
                          .make(),
                ctx_,
                game_model->kicks.get(),
                game_model->snares.get(),
                game_model->hats.get());
        break;
      case 2: {
        auto was_right =
            std::get<Quizz::Pause>(game_model->type).was_answer_right;
        panel = std::make_unique<PausePanel>(ctx_, was_right);
        break;
      }
      case 3:
        panel = nullptr;
        break;
      default: {}
    }
    addAndMakeVisible(panel.get());
    resized();
  }
public:
  explicit QuizzComponent(const lager::reader<Quizz::model>& model,
                          lager::context<Quizz::quizzAction> ctx)
      : game_model(model),
        step_index(model[&Quizz::model::type]
                        .xform(zug::map([](auto var) { return var.index();}))),
        ctx_(std::move(ctx)),

        current_score_label(
            "correct : ",
            game_model[&Quizz::model::score][&Quizz::Score::correct_anwsers]),
        total_score_label(
            "total : ",
            game_model[&Quizz::model::score][&Quizz::Score::total_answers]),

        step_title(game_model[&Quizz::model::type])
  {

    step_index.watch([this](const auto& new_step){
      updatePanel(new_step);
    });
    updatePanel(step_index.get());


    addAndMakeVisible(current_score_label);
    addAndMakeVisible(total_score_label);

    addAndMakeVisible(step_title);
  }

  void resized() override
  {
    auto totalBounds = getLocalBounds().reduced(10);

    auto titleHeight = 30;

    auto titleBounds = totalBounds.withHeight(titleHeight);
    step_title.setBounds(titleBounds);

    auto bounds = totalBounds.withTrimmedTop(titleHeight);

    int topHeight = 30;

    auto topBound = bounds.withHeight(topHeight);
    current_score_label.setBounds(topBound.withTrimmedLeft(topBound.getWidth()/2));
    total_score_label.setBounds(topBound.withTrimmedRight(topBound.getWidth()/2));


    auto listBound = bounds.withTrimmedTop(topHeight);

    if(panel)
     panel->setBounds(listBound);
  }

  void paint(juce::Graphics &g) override {}
};
