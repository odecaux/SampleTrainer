//
// Created by Octave on 25/12/2020.
//
#include <juce_gui_extra/juce_gui_extra.h>
#include "../../Samples/SampleInfos.h"
#include "Game.h"

namespace Quizz
{

model new_model(const std::vector<SampleInfos>& samples)
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
  return model{
      sampleContainer{std::move(kicks)},
      sampleContainer{std::move(snares)},
      sampleContainer{std::move(hats)}
  };
}


model changeSelectedSample(model current, selectSample action)
{
  auto index = action.index;


  if(action.type == SampleType::kick) {
    current.kicks.selected_index = index;
  } else if(action.type == SampleType::snare)
    current.snares.selected_index = index;
  else if(action.type == SampleType::hats)
    current.hats.selected_index = index;

  return current;
}

model answer(model current, answerQuestion action)
{
  assertm(std::holds_alternative<Question>(current.type), "invalid transition");
  auto state = std::get<Question>(current.type);

  if (action.hats_index == state.hats_index &&
      action.kick_index == state.kick_index &&
      action.snare_index == state.snare_index) {

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
  current.type =
      Question{rand() % (int)current.kicks.size(),
                     rand() % (int)current.snares.size(),
                     rand() % (int)current.hats.size()};

  current.kicks.selected_index = std::nullopt;
  current.snares.selected_index = std::nullopt;
  current.hats.selected_index = std::nullopt;

  return  current;
}

model update(model c, quizzAction a)
{
  return std::visit(lager::visitor{
          [&](struct selectSample a) {
                       if (std::holds_alternative<Auditioning>(c.type) ||
                           std::holds_alternative<Question>(c.type))
                         return changeSelectedSample(c, a);
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