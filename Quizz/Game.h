#pragma once

#include <variant>
#include <random>
//#include <lager/util.hpp>

#define assertm(exp, msg) assert(((void)msg, exp))

//model state
struct QuizzModel
{
  struct Idle{};
  struct Question{
    int kick_index;
    int snare_index;
    int hats_index;
  };
  struct Pause{};
  struct DisplayResults{};

  typedef std::variant<Idle,Question,Pause,DisplayResults> StateType;



  StateType type = Idle{};
  int correct_anwsers = 0;
  int total_answers = 0;
};

//actions
struct nextQuestion{};
struct leaveQuizz{};
struct answerQuestion{
  int kick_index;
  int snare_index;
  int hats_index;
};

typedef std::variant<answerQuestion, nextQuestion, leaveQuizz>
    quizzAction;


//updates
QuizzModel update(QuizzModel current, answerQuestion action)
{
  assertm(std::holds_alternative<QuizzModel::Question>(current.type), "invalid transition");
  auto state = std::get<QuizzModel::Question>(current.type);

  if (action.hats_index == state.hats_index &&
      action.kick_index == state.kick_index &&
      action.snare_index == state.snare_index) {

    current.correct_anwsers += 1;
    current.total_answers += 1;
  }
  else{
    current.total_answers += 1;
  }

  current.type = QuizzModel::Pause{};
  return current;
}


QuizzModel update(QuizzModel current, nextQuestion action)
{
  current.type = QuizzModel::Question{rand() % current.kick_count,
                                      rand() % current.snare_count,
                                      rand() % current.hats_count};
  return  current;
}

QuizzModel update(QuizzModel current, leaveQuizz action)
{
  current.type = QuizzModel::DisplayResults{};
  return current;
}

QuizzModel update(QuizzModel current, quizzAction a)
{

  return std::visit([&] (auto a) { return update(current, a); }, a);
}
