
#include <juce_core/juce_core.h>
#include "SampleInfos.h"
#include "SampleRepository.h"

void SampleRepository::removeSamples(const juce::SparseSet<int> &rowIndexes) {
  auto numRowsToDelete = rowIndexes.size();

  std::vector<int> sampleIndexes;
  for (auto i = 0; i < numRowsToDelete; ++i) {
    sampleIndexes.push_back(ids[rowIndexes[i]]);
  }

  // remove the samples
  for (auto i = 0; i < numRowsToDelete; ++i) {
    auto index_to_erase = sampleIndexes[i] - i;
    rows.erase(rows.begin() + index_to_erase);
  }

  numRows = numRows - numRowsToDelete;
  ids = std::vector<int>(numRows);
  std::ranges::generate(ids, [n = 0]() mutable { return n++; });
  sortByColumn(currentSortingColumn);
}

juce::String SampleRepository::getFieldAsString(Column column, int row) {
  auto id = ids[row];

  return [column, id, this] () -> juce::String {
    switch (column)
    {
    case Column::id:
      return std::to_string(id);
    case Column::path:
      return rows[id].getPath();
    case Column::type:
      return typeToString(rows[id].type);
    case Column::name:
      return rows[id].getName();
    case Column::rank:
      return juce::String(rows[id].rank);
    }
  }();
}

Column SampleRepository::idToColumn(int columnId) {
  switch (columnId) {
  case 1:
    return Column::id;
  case 2:
    return Column::path;
  case 3:
    return Column::name;
  case 4:
    return Column::type;
  case 5:
    return Column::rank;
  default:
    jassertfalse;
    return Column::id;
  }
}

void SampleRepository::sortByColumn(Column columnToSortBy) {
  currentSortingColumn = columnToSortBy;

  switch (columnToSortBy) {
  case Column::id:
    std::ranges::sort(ids);
    break;
  case Column::path:
    std::ranges::sort(ids, [this](int a, int b) {
      return rows[a].getPath().compareNatural(rows[a].getPath());
    });
    break;
  case Column::name:
    std::ranges::sort(ids, [this](int a, int b) {
      return rows[a].getName().compareNatural(rows[a].getName());
    });
    break;
  case Column::type:
    std::ranges::sort(
        ids, [this](int a, int b) { return rows[a].type > rows[b].type; });
    break;
  case Column::rank:
    std::ranges::sort(
        ids, [this](int a, int b) { return rows[a].rank > rows[b].rank; });
    break;
  }
}

juce::String SampleRepository::typeToString(SampleType type) {
  switch (type) {
  case SampleType::hats:
    return "hats";
  case SampleType::kick:
    return "kick";
  case SampleType::snare:
    return "snare";
  }
}