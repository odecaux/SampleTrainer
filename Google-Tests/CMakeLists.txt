# 'Google_test' is the subproject name
project(Google_tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(Google_Tests_run
        quizz_test.cpp
        quizz_component_test.cpp
        ../../Quizz/Game.cpp)
target_link_libraries(Google_Tests_run
        gtest
        gtest_main
        lager
        juce::juce_gui_extra
        juce::juce_audio_utils)