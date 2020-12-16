#include "gtest/gtest.h"
#include "../Quizz/Game.h"

TEST(testQuizzModel, init)
{
  auto model = QuizzModel{};
  ASSERT_TRUE(std::holds_alternative<QuizzModel::Idle>(model.type));
}

TEST(testQuizzModel, transitions)
{
  auto model = QuizzModel{1,1,1};
  ASSERT_TRUE(std::holds_alternative<QuizzModel::Idle>(model.type));

  model = update(model, nextQuestion{});
  ASSERT_TRUE(std::holds_alternative<QuizzModel::Question>(model.type));

  model = update(model, answerQuestion{0,0,0});
  ASSERT_TRUE(std::holds_alternative<QuizzModel::Pause>(model.type));
  ASSERT_EQ(model.correct_anwsers, 1);
  ASSERT_EQ(model.total_answers, 1);

  model = update(model, nextQuestion{});
  ASSERT_TRUE(std::holds_alternative<QuizzModel::Question>(model.type));

  auto model_leave = update(model, leaveQuizz{});
  ASSERT_TRUE(std::holds_alternative<QuizzModel::DisplayResults>(model_leave.type));

  model = update(model, answerQuestion{1,0,0});
  ASSERT_TRUE(std::holds_alternative<QuizzModel::Pause>(model.type));
  ASSERT_EQ(model.correct_anwsers, 1);
  ASSERT_EQ(model.total_answers, 2);

  model_leave = update(model, leaveQuizz{});
  ASSERT_TRUE(std::holds_alternative<QuizzModel::DisplayResults>(model_leave.type));
}


TEST(testQuizzModel, invalid_transitions)
{
  auto model_idle = QuizzModel{1,1,1};
  ASSERT_DEATH(update(model_idle, answerQuestion{}), "invalid transition");

  auto model_pause = QuizzModel{1,1,1, QuizzModel::Pause{}};
  ASSERT_DEATH(update(model_pause, answerQuestion{} ), "invalid transition");

  auto model_display = QuizzModel{1,1,1, QuizzModel::DisplayResults{}};
  ASSERT_DEATH(update(model_display, answerQuestion{} ), "invalid transition");
}