cmake_minimum_required(VERSION 3.17)
project(SAMPLE_TRAINER VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -EHsc)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(JUCE)
find_package(Lager)
find_package(Immer)
find_package(Zug)

juce_add_gui_app(SampleTrainer PRODUCT_NAME "Sample Trainer")

target_sources(SampleTrainer PRIVATE
        Main.cpp
        Samples/SampleBufferCache.cpp
        Quizz/Sampler.cpp
        Samples/SampleRepository.cpp
        SampleSelector/SampleSelectorPanel.cpp
        Quizz/QuizzComponent.cpp
        Quizz/Game.cpp)

target_compile_definitions(SampleTrainer PRIVATE
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SampleTrainer,JUCE_PROJECT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SampleTrainer,JUCE_VERSION>")

target_link_libraries(SampleTrainer PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_utils
        lager
        immer
        zug)

add_subdirectory(Google-Tests)
