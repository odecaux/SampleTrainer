//
// Created by Octave on 25/12/2020.
//
#include <juce_gui_extra/juce_gui_extra.h>
#include "../../Samples/SampleInfos.h"
#include "Game.h"

namespace Quizz
{

model model::new_model(const std::vector<SampleInfos>& samples)
{
  std::vector<SampleInfos> kicks;
  std::vector<SampleInfos> snares;
  std::vector<SampleInfos> hats;


  for(const auto& sample : samples)
  {
    if(sample.type == SampleType::kick)
      kicks.push_back(sample);
    if(sample.type == SampleType::snare)
      snares.push_back(sample);
    if(sample.type == SampleType::hats)
      hats.push_back(sample);
  }

  assertm(!kicks.empty() && !snares.empty() && !hats.empty(), "should have non-empty question vectors");
  return model{
      std::move(kicks),
      std::move(snares),
      std::move(hats)
  };
}


model changeQuestionSelectedSample(model current, selectSample action)
{
  assertm(std::holds_alternative<Question>(current.type), "invalid transition");
  auto state = std::get<Question>(current.type);

  auto index = action.index;

  if(action.type == SampleType::kick)
    state.answer_kick_index = index;
  else if (action.type == SampleType::snare)
    state.answer_snare_index = index;
  else if (action.type == SampleType::hats)
    state.answer_hats_index = index;

  current.type = state;
  return current;
}

model changeAuditionSelectedSample(model current, selectSample action)
{
  assertm(std::holds_alternative<Auditioning>(current.type), "invalid transition");
  auto state = std::get<Auditioning>(current.type);

  auto index = action.index;

  if(action.type == SampleType::kick)
    state.kick_index = index;
  else if (action.type == SampleType::snare)
    state.snare_index = index;
  else if (action.type == SampleType::hats)
    state.hats_index = index;

  current.type = state;
  return current;
}

model answer(model current, answerQuestion action)
{
  assertm(std::holds_alternative<Question>(current.type), "invalid transition");
  auto state = std::get<Question>(current.type);

  assertm(state.answer_snare_index && state.answer_kick_index && state.answer_hats_index, "les 3 colonnes doivent être sélectionnées");


  if (state.question_kick_index == state.answer_kick_index &&
      state.question_snare_index == state.answer_snare_index &&
      state.question_hats_index == state.answer_hats_index) {

    current.score.correct_anwsers += 1;
    current.score.total_answers += 1;
  }
  else{
    current.score.total_answers += 1;
  }

  current.type = Pause{};
  return current;
}

model next(model current)
{
  auto state =  Question{rand() % (int)current.kicks->size(),
                     rand() % (int)current.snares->size(),
                     rand() % (int)current.hats->size()};

  state.answer_kick_index = std::nullopt;
  state.answer_snare_index = std::nullopt;
  state.answer_hats_index = std::nullopt;

  current.type = state;
  return  current;
}

model update(model c, quizzAction a)
{
  return std::visit(lager::visitor{
          [&](struct selectSample a) {
                       if (std::holds_alternative<Auditioning>(c.type))
                         return changeAuditionSelectedSample(c, a);
                       else if (std::holds_alternative<Question>(c.type))
                         return changeQuestionSelectedSample(c, a);
                       else
                         assert(false);
                     },
          [&](struct answerQuestion a) { return answer(c, a); },
          [&](struct nextQuestion) { return next(c); },
          [&](struct leaveQuizz) {
            c.type = DisplayResults{};
            return c;
          }},
      a);
}

}