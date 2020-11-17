cmake_minimum_required(VERSION 3.12)

project(SAMPLE_TRAINER VERSION 0.0.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHsc")
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(JUCE)

juce_add_gui_app(SampleTrainer
    PRODUCT_NAME "Sample Trainer")     

target_sources(SampleTrainer PRIVATE
    Main.cpp
    SampleBufferCache.cpp)

target_compile_definitions(SampleTrainer PRIVATE
    JUCE_WEB_BROWSER=0  
    JUCE_USE_CURL=0     
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SampleTrainer,JUCE_PROJECT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SampleTrainer,JUCE_VERSION>")

target_link_libraries(SampleTrainer PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_utils)
