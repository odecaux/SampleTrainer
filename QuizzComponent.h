#pragma once

#include <algorithm>
#include <memory>

class SamplerSource : public juce::AudioSource
{
public:
    SamplerSource()
    {
        std::vector<Note> notes{

                {hats,0},
                {hats,1},
                {hats,2},
                {hats,3},
                {hats,4},
                {hats,5},
                {hats,6},
                {hats,7},
                {hats,8},
                {hats,9},
                {hats,10},
                {hats,11},
                {hats,12},
                {hats,13},
                {hats,14},
                {hats,15},
                {kick,0}, {snare, 4}, {kick, 7},{kick, 8}, {snare, 12}
        };
        sequence = std::make_unique<Sequence>(Sequence{16, 16, std::move(notes)});
    }
    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override
    {
        sampleRate = newSampleRate;
        numSamples = samplesPerBlockExpected;

        sequence->prepareToPlay(samplesPerBlockExpected, newSampleRate);
        sampler.setCurrentPlaybackSampleRate(newSampleRate);
        midiCollector.reset (newSampleRate); // [10]
    }

    void getNextAudioBlock (const AudioSourceChannelInfo &bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();


        juce::MidiBuffer midiMessages;
        //midiCollector.removeNextBlockOfMessages (midiMessages, bufferToFill.numSamples); // [11]

        sequence->renderNextBlock(midiMessages);
        sampler.renderNextBlock (*bufferToFill.buffer, midiMessages,
                                 bufferToFill.startSample, bufferToFill.numSamples);
    }

    void releaseResources() override
    {

    }

    void addSample(const SampleInfos& sampleInfos, SampleBufferCache& cache)
    {
        auto buffer = cache.getOrCreateSampleBuffer(sampleInfos);
        //TODO setup types
        sampler.addVoice(new MySamplerVoice(buffer, 60 + sampleInfos.type));
        mSampleCount++;
    }

    void clearSamples()
    {
        sampler.clearVoices();
    }

    juce::MidiMessageCollector& getMidiCollector() { return midiCollector; }


    void play() { sequence->play(); }
    void stop() { sequence->stop(); }

private:

    juce::MidiMessageCollector midiCollector;
    MySampler sampler;

    std::unique_ptr<Sequence> sequence;
    int numSamples{};
    double sampleRate{};
    int mSampleCount{};
    int time{};
};

class QuizzComponent : public juce::Component
{
public:
    explicit QuizzComponent(juce::AudioDeviceManager& dm, SampleBufferCache& cache)
        : deviceManager(dm), kickModel(*this), snareModel(*this), hatsModel(*this), cache(cache)
    {


        kickList.setModel(&kickModel);
        snareList.setModel(&snareModel);
        hatsList.setModel(&hatsModel);

        addAndMakeVisible(kickList);
        addAndMakeVisible(snareList);
        addAndMakeVisible(hatsList);
        addAndMakeVisible(answerButton);

        deviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&samplerSource);

        setupMidi();
    }

    ~QuizzComponent() override
    {
        audioSourcePlayer.setSource (nullptr);
        deviceManager.removeAudioCallback(&audioSourcePlayer);
    }

    void paint(Graphics &g) override
    {
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10);

        int bottomHeight = 40;

        auto listWidth = bounds.getWidth() / 3;
        auto listBound = bounds
                .withTrimmedBottom(bottomHeight)
                .withWidth(listWidth);

        kickList.setBounds(listBound
                                   .reduced(3));
        snareList.setBounds((listBound + Point(listWidth, 0))
                                    .reduced(3));
        hatsList.setBounds((listBound + Point(listWidth * 2, 0))
                                  .reduced(3));

        auto buttonRow = bounds.withTrimmedTop(listBound.getHeight()).reduced(3);
        answerButton.setBounds(buttonRow.withTrimmedLeft(buttonRow.getWidth() - 120));
    }

    void addSamples(std::vector<SampleInfos>&& samples)
    {
        for(SampleInfos& sample : samples)
        {
            if(sample.type == kick){
                kickModel.addSample(sample);
                kickList.updateContent();
            }
            else if (sample.type == snare)
            {
                snareModel.addSample(sample);
                snareList.updateContent();
            }
            else if (sample.type == hats)
            {
                hatsModel.addSample(sample);
                hatsList.updateContent();
            }
        }

        initGame();
    }

    void play() { samplerSource.play(); }
    void stop() { samplerSource.stop(); }

private:
    class SampleListModel : public juce::ListBoxModel
    {
    public:

        explicit SampleListModel(juce::Component& parent) : parent(parent){

        }

        SampleListModel() = delete;

        void addSample(SampleInfos& sample)
        {
            samples.push_back(std::move(sample));
        }
        int getNumRows() override
        {
            return samples.size();
        }

        void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
        {

            auto alternateColour = parent.getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
                    .interpolatedWith(parent.getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
            if (rowIsSelected)
                g.fillAll(juce::Colours::lightblue);
            else if (rowNumber % 2)
                g.fillAll(alternateColour);

            g.setColour(parent.getLookAndFeel().findColour(juce::ListBox::textColourId));
            g.setFont(font);

            if (rowNumber < getNumRows()) {
                auto cellText = samples[rowNumber].file.getFileName();
                g.drawText(cellText, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            }
        }


        SampleInfos& getSelectedSample() {
            jassert(lastRow != -1);
            jassert(lastRow < samples.size());
            return samples[lastRow];
        }
        SampleInfos& getSampleAt(int rowId) {
            jassert(rowId >= 0 && rowId < samples.size());
            return samples[rowId];
        }

        void selectedRowsChanged (int lastRowSelected) override
        {
            lastRow = lastRowSelected;
        }


    private:
        int lastRow = -1;
        std::vector<SampleInfos> samples{};
        juce::Font font{14.0f};
        juce::Component& parent;
    };

    juce::AudioSourcePlayer audioSourcePlayer;
    SamplerSource samplerSource;
    juce::AudioDeviceManager& deviceManager;
    SampleBufferCache& cache;

    juce::ListBox kickList;
    juce::ListBox snareList;
    juce::ListBox hatsList;
    juce::TextButton answerButton{"answer"};

    SampleListModel kickModel;
    SampleListModel snareModel;
    SampleListModel hatsModel;

    SampleInfos* usedKick;
    SampleInfos* usedSnare;
    SampleInfos* usedHats;


    void initGame()
    {
        answerButton.onClick = [this]{
            checkAnswers();
        };
        nextQuestion();
    }

    class ModalReturn : public juce::ModalComponentManager::Callback
    {
    public:
        explicit ModalReturn(QuizzComponent& parent) : parent(parent)
        {
        }
        QuizzComponent& parent;

        void modalStateFinished(int returnValue) override
        {
            parent.nextQuestion();
        }
    };

    void checkAnswers()
    {
        stop();
        if(usedHats && usedKick && usedSnare )
        {
            if(*usedHats == hatsModel.getSelectedSample()
               && *usedKick == kickModel.getSelectedSample()
               && *usedSnare == snareModel.getSelectedSample())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::NoIcon, "Nice",
                                                        "Nice",
                                                        "OK",
                                                        this,
                                                        new ModalReturn{*this});
            }
            else
            {

                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::NoIcon, "Bad",
                                                        "Wrongs Samples",
                                                        "OK",
                                                        this,
                                                        new ModalReturn{*this});
            }
        }
    }

    void nextQuestion()
    {
        samplerSource.clearSamples();
        kickList.deselectAllRows();
        snareList.deselectAllRows();
        hatsList.deselectAllRows();

        auto randomSample = [](SampleListModel& model) -> SampleInfos&
        {
            auto id = Random().nextInt(model.getNumRows());
            return model.getSampleAt(id);
        };

        usedKick = &randomSample(kickModel);
        usedSnare = &randomSample(snareModel);
        usedHats = &randomSample(hatsModel);

        samplerSource.addSample(*usedKick, cache);
        samplerSource.addSample(*usedSnare, cache);
        samplerSource.addSample(*usedHats, cache);


        play();
    }

    void endGame()
    {
        stop();
    }


    void setupMidi ()
    {
        auto list = juce::MidiInput::getAvailableDevices();

        for(auto& input : list){
            if (! deviceManager.isMidiInputDeviceEnabled (input.identifier))
                deviceManager.setMidiInputDeviceEnabled (input.identifier, true);
            deviceManager.addMidiInputDeviceCallback (input.identifier, &samplerSource.getMidiCollector());
        }
    }
};
