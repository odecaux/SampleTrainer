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

struct Idle{};
struct Question{int kick_index; int snare_index; int hats_index;};
struct Pause{};
struct DisplayResults{};

typedef std::variant<Idle,
                     Question,
                     Pause,
                     DisplayResults>
        StepType;


struct sampleContainer{
  immer::box<std::vector<SampleInfos>> samples;
  std::optional<int> selected_index;

  [[nodiscard]] int size() const { return samples->size();}
};


struct Score{
  int correct_anwsers = 0;
  int total_answers = 0;
};


//model
struct model {
  sampleContainer kicks;
  sampleContainer snares;
  sampleContainer hats;

  StepType type = Idle{};

  Score score{};
};

model new_model(const std::vector<SampleInfos>&);


//actions
struct nextQuestion{};
struct selectSample{ SampleType type; int index; };
struct answerQuestion{int kick_index; int snare_index; int hats_index; };
struct leaveQuizz{};

typedef std::variant<selectSample,
                     answerQuestion,
                     nextQuestion,
                     leaveQuizz>
    quizzAction;

model changeSelectedSample(model, selectSample);
model next(model, nextQuestion);
model answer(model, answerQuestion);
model leave(model);
model update(model, quizzAction);

using boost::fusion::operator!=;
using boost::fusion::operator==;
}

BOOST_FUSION_ADAPT_STRUCT(Quizz::Idle)
BOOST_FUSION_ADAPT_STRUCT(Quizz::Question)
BOOST_FUSION_ADAPT_STRUCT(Quizz::DisplayResults)
BOOST_FUSION_ADAPT_STRUCT(Quizz::Pause)

BOOST_FUSION_ADAPT_STRUCT(Quizz::Score, correct_anwsers, total_answers);
BOOST_FUSION_ADAPT_STRUCT(Quizz::sampleContainer,samples, selected_index);
BOOST_FUSION_ADAPT_STRUCT(Quizz::model, kicks, snares, hats, type, score);