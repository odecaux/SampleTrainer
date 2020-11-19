#pragma once

enum SampleType { kick = 1, snare = 2, hats = 3 };
enum class Column {
  id,
  path,
  name,
  type,
  rank,
};

class SampleInfos {
public:
  SampleInfos() = delete;
  SampleInfos(juce::File file, SampleType type, int rank)
      : type(type), file(std::move(file)), rank(rank) {}

  SampleInfos(const SampleInfos &other) = default;
  SampleInfos(SampleInfos &&other) noexcept = default;
  SampleInfos &operator=(const SampleInfos &other) = default;
  SampleInfos &operator=(SampleInfos &&other) = default;

  [[nodiscard]] juce::String getName() const {
    return file.getFileNameWithoutExtension();
  }
  [[nodiscard]] juce::String getPath() const { return file.getFullPathName(); }

  bool operator==(const SampleInfos &other) const { return other.file == file; }

  juce::File file;
  SampleType type;
  int rank;

  [[nodiscard]] juce::String serialize() const {
    juce::String out;
    out << "\"" << getPath() << "\"," << type << "," << rank;
    return out;
  }

  static std::unique_ptr<SampleInfos> deserialize(const juce::String &line) {
    juce::StringArray tokens;
    tokens.addTokens(line, ",", "\"");
    if (tokens.size() != 3)
      return nullptr;
    auto file = juce::File(tokens[0].removeCharacters("\""));
    auto type = tokens[1].getIntValue();
    auto rank = tokens[2].getIntValue();
    if (!file.exists())
      return nullptr;
    return std::make_unique<SampleInfos>(file, SampleType(type), rank);
  }
};