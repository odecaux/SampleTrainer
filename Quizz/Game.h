#pragma once

#include <variant>
#include <random>
#include <optional>
#include <lager/util.hpp>
#include <immer/flex_vector.hpp>
#include <immer/box.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/comparison.hpp>

#define assertm(exp, msg) assert(((void)msg, exp))


namespace Quizz
{

struct Auditioning {
  int kick_index = 0;
  int snare_index = 0;
  int hats_index = 0;
  bool playing = true;};

struct Question{
  int question_kick_index;
  int question_snare_index;
  int question_hats_index;

  std::optional<int> answer_kick_index;
  std::optional<int> answer_snare_index;
  std::optional<int> answer_hats_index;


};


struct Pause{
  bool was_answer_right;
};
struct DisplayResults{};

typedef std::variant<Auditioning,
                     Question,
                     Pause,
                     DisplayResults>
        StepType;

//model

struct Score{
  int correct_anwsers = 0;
  int total_answers = 0;
};


struct model {
  immer::box<std::vector<SampleInfos>> kicks;
  immer::box<std::vector<SampleInfos>> snares;
  immer::box<std::vector<SampleInfos>> hats;

  StepType type = Auditioning{};

  Score score{};

  explicit model() = delete;


  static model new_model(const std::vector<SampleInfos>&);

private:

  model(std::vector<SampleInfos>&& kicks,
  std::vector<SampleInfos>&& snares,
  std::vector<SampleInfos>&& hats)
  : kicks(std::move(kicks)),
        snares(std::move(snares)),
        hats(std::move(hats)) {}
};



//actions
struct nextQuestion{};
struct selectSample{ SampleType type; int index; };
struct answerQuestion{};
struct leaveQuizz{};

typedef std::variant<selectSample,
                     answerQuestion,
                     nextQuestion,
                     leaveQuizz>
    quizzAction;

model answer(model, answerQuestion);
model update(model, quizzAction);

using boost::fusion::operator!=;
using boost::fusion::operator==;
}

BOOST_FUSION_ADAPT_STRUCT(Quizz::Auditioning, kick_index, hats_index, snare_index, playing)
BOOST_FUSION_ADAPT_STRUCT(Quizz::Question, question_kick_index, question_hats_index, question_snare_index,
                          answer_kick_index, answer_snare_index, answer_hats_index)
BOOST_FUSION_ADAPT_STRUCT(Quizz::DisplayResults)
BOOST_FUSION_ADAPT_STRUCT(Quizz::Pause, was_answer_right)

BOOST_FUSION_ADAPT_STRUCT(Quizz::Score, correct_anwsers, total_answers);
BOOST_FUSION_ADAPT_STRUCT(Quizz::model, kicks, snares, hats, type, score);