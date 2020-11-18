#pragma once
#include "Samples/SampleBufferCache.h"
#include "Samples/SampleRepository.h"
#include "SampleSelector/SampleSelectorAudio.h"
#include "SampleSelector/SampleSelectorComponent.h"

#include "Samples/Sampler.h"
#include "Quizz/Sequencer.h"
#include "Quizz/QuizzComponent.h"

using namespace juce;

class MainComponent : public Component
{
public:
    //==============================================================================
    MainComponent()
    {
        mFormatManager.registerBasicFormats();
        deviceManager.initialiseWithDefaultDevices(0,2);

        sampleLoader = std::make_unique<SampleBufferCache>(mFormatManager);

        sampleSelector = std::make_unique<SampleSelectorComponent>(deviceManager, *sampleLoader);
        sampleSelector->setOnStartButtonClick([&](std::vector<SampleInfos>&& samples) {
            showComponent(quizz.get());
            quizz->addSamples(std::move(samples));
        });

        showComponent(sampleSelector.get());
        sampleSelector->loadRepositoryFromDisk();

        quizz = std::make_unique<QuizzComponent>(deviceManager, *sampleLoader);

        setSize(600, 400);
    }

    ~MainComponent() override
    {
        sampleSelector->saveRepositoryToDisk();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
    }

    void resized() override
    {
        if(panelComponent)
            panelComponent->setBounds(getLocalBounds());
    }


private:
    AudioFormatManager mFormatManager;
    juce::AudioDeviceManager deviceManager;

    std::unique_ptr<SampleSelectorComponent> sampleSelector;
    std::unique_ptr<QuizzComponent> quizz;

    juce::WeakReference<Component> panelComponent;

    std::unique_ptr<SampleBufferCache> sampleLoader;


    void showComponent(juce::Component* newPanel)
    {
        if(newPanel != panelComponent)
        {
            if(panelComponent != nullptr)
            {
                panelComponent->setVisible(false);
                removeChildComponent (panelComponent);
            }
            panelComponent = newPanel;
            if (panelComponent != nullptr)
            {
                addChildComponent(panelComponent);
                newPanel->setVisible(true);
            }
            repaint();
        }
        resized();
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
