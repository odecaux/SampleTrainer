#pragma once
#include <future>


struct SampleBuffer {
  SampleBuffer(juce::AudioBuffer<float> &&buffer,
               const double d)
      : buffer(std::move(buffer)), sourceSampleRate(d) {}

  juce::AudioBuffer<float> buffer;
  double sourceSampleRate;
};


typedef std::shared_ptr<SampleBuffer> SampleBufferPtr;

class SampleBufferCache : juce::Timer {
  typedef std::function<void(SampleBufferPtr)> Callback;

  static bool isReady(const std::future<SampleBufferPtr> &f)
  {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  }

public:
  explicit SampleBufferCache(juce::AudioFormatManager &formatManager)
      : formatManager(formatManager) {
    //TODO on peut faire mieux
    startTimer(100);
  }

  void timerCallback() override
  {

    std::vector<std::string> to_remove;


    //TODO synchronisation
    for(auto& pair : futures)
    {
      auto &future = pair.second.first;
      auto &callback = pair.second.second;
      auto &file = pair.first;
      if(isReady(future))
      {
        auto ptr = future.get();
        sampleBuffers.emplace(file, ptr);
        to_remove.push_back(file);
        callback(ptr);
      }
    }

    for(const auto& file : to_remove)
      futures.erase(file);

  }


  SampleBufferPtr getOrCreateSampleBuffer(const SampleInfos &sampleInfos);

  void getOrCreateSampleBufferAsync(const SampleInfos &sampleInfos,const Callback& callback );

  void checkForSamplesToFree();

private:
  SampleBufferPtr createBuffer(const SampleInfos &sampleInfos);

  std::unordered_map<std::string,std::pair<std::future<SampleBufferPtr>, Callback>> futures;
  std::map<juce::File, SampleBufferPtr> sampleBuffers{};
  juce::AudioFormatManager &formatManager;
};