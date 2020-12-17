#pragma once

#include <variant>
#include <random>
#include <lager/util.hpp>
#include <immer/flex_vector.hpp>

#define assertm(exp, msg) assert(((void)msg, exp))

namespace Quizz
{
//state
struct Idle{};
struct Question{int kick_index; int snare_index; int hats_index;};
struct Pause{};
struct DisplayResults{};

typedef std::variant<Idle,
                     Question,
                     Pause,
                     DisplayResults> StateType;

//model
struct model {
  immer::flex_vector<SampleInfos> kicks;
  immer::flex_vector<SampleInfos> snares;
  immer::flex_vector<SampleInfos> hats;

  StateType type = Idle{};
  int correct_anwsers = 0;
  int total_answers = 0;
};

//actions
struct nextQuestion{};
struct leaveQuizz{};
struct answerQuestion{int kick_index; int snare_index; int hats_index; };

typedef std::variant<answerQuestion,
                     nextQuestion,
                     leaveQuizz> quizzAction;

//updates
model update(model current, answerQuestion action)
{
  assertm(std::holds_alternative<Question>(current.type), "invalid transition");
  auto state = std::get<Question>(current.type);

  if (action.hats_index == state.hats_index &&
      action.kick_index == state.kick_index &&
      action.snare_index == state.snare_index) {

    current.correct_anwsers += 1;
    current.total_answers += 1;
  }
  else{
    current.total_answers += 1;
  }

  current.type = Pause{};
  return current;
}

model update(model current, nextQuestion action)
{
  current.type =
      Question{rand() % (int)current.kicks.size(),
               rand() % (int)current.snares.size(),
               rand() % (int)current.hats.size()};
  return  current;
}

model update(model current, leaveQuizz action)
{
  current.type = DisplayResults{};
  return current;
}

model update(model current, quizzAction a)
{
  return std::visit([&] (auto a) { return update(current, a); }, a);
}

}

