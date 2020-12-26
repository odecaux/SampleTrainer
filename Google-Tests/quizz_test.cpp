
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "../Samples/SampleInfos.h"
#include "../Quizz/Sequencer.h"
#include "../Quizz/Game.h"
#include "gtest/gtest.h"
#include <lager/store.hpp>
#include <lager/event_loop/manual.hpp>

using namespace Quizz;

TEST(testQuizzModel, model_init)
{
  auto model = Quizz::model{};
  ASSERT_TRUE(std::holds_alternative<Idle>(model.type));
  ASSERT_EQ(model.score.correct_anwsers,0);
  ASSERT_EQ(model.score.total_answers,0);
}

TEST(testQuizzModel, container_default_init)
{
  auto container = sampleContainer{};
  ASSERT_EQ(container.selected_index, std::nullopt);
  ASSERT_EQ(container.size(), 0);
}

TEST(testQuizzModel, container_init)
{

  auto container =
      sampleContainer{
      .samples = {SampleInfos{juce::File{"coucou"}, SampleType::hats, 0},
                  SampleInfos{juce::File{"coucou"}, SampleType::hats, 0}},
      .selected_index = 1};

  ASSERT_EQ(container.selected_index, 1);
  ASSERT_EQ(container.size(), 2);
}

TEST(testQuizzModel, container_copy)
{
  auto vec = std::vector<SampleInfos>{SampleInfos{juce::File{"coucou"}, SampleType::hats, 0},
                                      SampleInfos{juce::File{"wsh"}, SampleType::hats, 0}};
  auto container = sampleContainer{ vec };

  ASSERT_EQ(container.size(), 2);

  auto container2 = container;

  container.samples = {SampleInfos{juce::File{"ehoh"}, SampleType::hats, 0},
                       SampleInfos{juce::File{"hihi"}, SampleType::hats, 0}};

  ASSERT_EQ(container.samples.get()[0].getName(), "ehoh");
  ASSERT_EQ(container2.samples.get()[0].getName(), "coucou");
}




/*
TEST(testQuizzModel, transitions)
{
  auto model = Quizz::model{};
  ASSERT_TRUE(std::holds_alternative<Idle>(model.type));

  model = update(model, nextQuestion{});
  ASSERT_TRUE(std::holds_alternative<Question>(model.type));

  model = update(model, answerQuestion{0,0,0});
  ASSERT_TRUE(std::holds_alternative<Pause>(model.type));
  ASSERT_EQ(model.score.correct_anwsers, 1);
  ASSERT_EQ(model.score.total_answers, 1);

  model = update(model, nextQuestion{});
  ASSERT_TRUE(std::holds_alternative<Question>(model.type));

  auto model_leave = update(model, leaveQuizz{});
  ASSERT_TRUE(std::holds_alternative<DisplayResults>(model_leave.type));

  model = update(model, answerQuestion{1,0,0});
  ASSERT_TRUE(std::holds_alternative<Pause>(model.type));
  ASSERT_EQ(model.score.correct_anwsers, 1);
  ASSERT_EQ(model.score.total_answers, 2);

  model_leave = update(model, leaveQuizz{});
  ASSERT_TRUE(std::holds_alternative<DisplayResults>(model_leave.type));
}*/


TEST(testQuizzModel, diff)
{
  auto vec = sampleContainer{{SampleInfos{{}, {}, {}}, SampleInfos{{}, {}, {}}}};

  auto model = Quizz::model{vec, vec, vec };

  auto store = lager::make_store<quizzAction>(model,
                                              Quizz::update,
                                              lager::with_manual_event_loop{});

  auto watcher = store[&model::type].make();
  watcher.watch([prev=store->type] (auto curr) mutable {
    ASSERT_NE(prev, curr);
    prev = curr;
  });

  store.dispatch(nextQuestion{});
  store.dispatch(selectSample{SampleType::kick, 0});
  store.dispatch(leaveQuizz{});

}

TEST(testQuizzModel, index_diff)
{
  lager::cursor<sampleContainer>  vec = lager::make_state(sampleContainer{},
                               lager::automatic_tag{});

  lager::cursor<std::optional<int>> watcher = vec[&sampleContainer::selected_index];

  bool was_set = false;
  vec.watch([&was_set](auto new_vec){
    if(new_vec.selected_index == 1)
      was_set = true;
  });

  watcher.update([](auto in){ return 1;});

  ASSERT_TRUE(was_set);

}

TEST(testLager, merge)
{
  auto index = lager::make_state(0);
  auto vec = lager::make_state(std::vector<int>{0,1});
  auto x = with(index, vec).make();

  ASSERT_EQ(x.get(), std::make_tuple(0, std::vector<int>{0,1}));

}

TEST(testLager, mapSimple)
{
  auto index = lager::make_state(0);
  auto vec = lager::make_state(0.1f);
  auto x = with(index, vec).xform(zug::map(
      [](int a, float b) { return a + b; })).make();
  ASSERT_FLOAT_EQ(x.get(), 0.1f);
}
TEST(testLager, mapIndex)
{
  auto index = lager::make_state(0);
  auto vec = lager::make_state(std::vector<int>{5,10,15});
  auto x = with(index, vec).xform(zug::map(
      [](int a, std::vector<int> b) { return b[a]; })).make();
  ASSERT_EQ(x.get(), 5);

  index.set(1);
  commit(index);
  ASSERT_EQ(x.get(), 10);
}

/*

TEST(testQuizzModel, invalid_transitions)
{
  auto model_idle = model{1,1,1};
  ASSERT_DEATH(update(model_idle, answerQuestion{}), "invalid transition");

  auto model_pause = model{1,1,1, Pause{}};
  ASSERT_DEATH(update(model_pause, answerQuestion{} ), "invalid transition");

  auto model_display = model{1,1,1, DisplayResults{}};
  ASSERT_DEATH(update(model_display, answerQuestion{} ), "invalid transition");
}
*/


static lager::reader<std::optional<sequenceSamples>> toSelected(const lager::reader<Quizz::model>& model)
{
  using namespace lager;
  using namespace lager::lenses;

  auto selector =
      [](const Quizz::sampleContainer& cont) -> std::optional<SampleInfos> {
        if (cont.selected_index)
          return std::make_optional(cont.samples.get()[cont.selected_index.value()]);
        else
          return std::nullopt;
      };

  auto kick = model[&Quizz::model::kicks].xform(zug::map(selector)).make();
  auto snare = model[&Quizz::model::snares].xform(zug::map(selector)).make();
  auto hat = model[&Quizz::model::hats].xform(zug::map(selector)).make();

  return with(kick,snare,hat).xform(
      zug::map([](const auto &kick, const auto &snare, const auto &hat) -> std::optional<sequenceSamples>{
        if (kick && snare && hat)
          return std::make_optional(sequenceSamples{kick.value(), snare.value(),
                                        hat.value()});
        else
          return std::nullopt;
      }));
}


/*
TEST(quizzTest, selectedSample)

{

  auto vec = sampleContainer{{SampleInfos{{}, {}, {}}, SampleInfos{{}, {}, {}}}};

  auto model = Quizz::model{vec, vec, vec };

  auto store = lager::make_store<quizzAction>(model,
                                              Quizz::update,
                                              lager::with_manual_event_loop{});

  auto selector =
      [](const Quizz::sampleContainer& cont) -> std::optional<SampleInfos> {
        if (cont.selected_index)
          return std::make_optional(cont.samples.get()[cont.selected_index.value()]);
        else
          return std::nullopt;
      };

  auto watcher = store[&Quizz::model::kicks].xform(zug::map(selector)).make();

  bool once = false;
  watcher.watch([prev = watcher.get(), &once] (auto curr) mutable {
    once = true;
    ASSERT_TRUE(curr);
    ASSERT_NE(curr, prev);
    prev = curr;
  });

  auto watcher_all = toSelected(store.make()).make();

  bool once_all = false;
  watcher_all.watch([prev = watcher_all.get(), &once_all] (auto curr) mutable {
    once_all = true;
    ASSERT_TRUE(curr);
    ASSERT_NE(curr, prev);
    prev = curr;
  });

  auto selectedKick = store->kicks.selected_index;

  store.dispatch(nextQuestion{});
  store.dispatch(selectSample{SampleType::kick, 0});
  store.dispatch(selectSample{SampleType::hats, 0});
  store.dispatch(selectSample{SampleType::snare, 0});
  store.dispatch(leaveQuizz{});
  auto selectedNewKick = store->kicks.selected_index;

  ASSERT_NE(selectedKick, selectedNewKick);
  ASSERT_TRUE(once);
  ASSERT_TRUE(once_all);
}*/
