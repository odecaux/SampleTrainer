#include <utility>

//
// Created by Octave on 05/11/2020.
//
#pragma once

// TODO le déplacer dans mainComponent
// TODO séparer la gestion des Samples, de la gestion du ListBoxModel,
// concrètement

class SampleRepository {

public:
  SampleRepository() = default;
  SampleInfos &getSampleInfos(int id) { return rows[id]; }

  void createSample(const juce::File &file) {
    // TODO il faudrait checker si le reader peut l'ouvrir ptn
    if (file.existsAsFile()) {
      addSample({file, kick, 0});
    }
  }

  void addSample(SampleInfos &&sample) {
    if (!existsInRepository(sample)) {
      ids.push_back(numRows);
      rows.push_back(std::move(sample));
      numRows++;
    }
  }

  void removeSamples(const juce::SparseSet<int> &rowIndexes);

  [[nodiscard]] int getNumRows() const { return numRows; }

  juce::String getFieldAsString(Column column, int row);

  static Column idToColumn(int columnId);

  void sortByColumnId(int columnId) { sortByColumn(idToColumn(columnId)); }

  void sortByColumn(Column columnToSortBy);

  void setSampleType(int row, SampleType type) { rows[ids[row]].type = type; }

  SampleType getSampleType(int row) { return rows[ids[row]].type; }

  juce::String serialize() {
    return std::accumulate(std::next(rows.begin()),
                    rows.end(),
                    rows[0].serialize(),
                    [](juce::String a, const SampleInfos &row){
                                         return std::move(a) + row.serialize();
                                       });
  }

private:
  std::vector<int> ids;
  std::vector<SampleInfos> rows;

  int numRows = 0;
  Column currentSortingColumn{};

  bool existsInRepository(const SampleInfos &sampleToTest) {
    return std::ranges::any_of(rows,
                               [&](auto &i) { return i == sampleToTest; });
  }

  static juce::String typeToString(SampleType type);
};
