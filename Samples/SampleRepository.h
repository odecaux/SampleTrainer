//
// Created by Octave on 05/11/2020.
//
#pragma once

#include <juce_core/juce_core.h>

enum SampleType  {kick = 1, snare = 2, hats = 3};
enum Column
{
    id, path, name, type, rank,
};

class SampleInfos
{
public:
    SampleInfos() = delete;
    SampleInfos(juce::File file, SampleType type,  int rank)
    : type(type), file(std::move(file)), rank(rank) {}

    SampleInfos(const SampleInfos& other) = default;
    SampleInfos(SampleInfos&& other) noexcept = default;
    SampleInfos& operator=(const SampleInfos& other) = default;
    SampleInfos& operator=(SampleInfos&& other) = default;

    [[nodiscard]] juce::String getName() const {return file.getFileNameWithoutExtension(); }
    [[nodiscard]] juce::String getPath() const {return file.getFullPathName(); }

    bool operator==(const SampleInfos& other) const{ return other.file == file; }

    juce::File file;
    SampleType type;
    int rank;

    [[nodiscard]] juce::String serialize() const
    {
        juce::String out;
        out <<"\""<< getPath() <<"\","<<type<<","<<rank;
        return out;
    }

    static std::unique_ptr<SampleInfos> deserialize(const juce::String& line)
    {
                juce::StringArray tokens;
                tokens.addTokens(line, ",","\"");
                if(tokens.size() != 3)
                    return nullptr;
                auto file = juce::File(tokens[0].removeCharacters("\""));
                auto type = tokens[1].getIntValue();
                auto rank = tokens[2].getIntValue();
                if(!file.exists())
                    return nullptr;
                return std::make_unique<SampleInfos>(file, SampleType(type), rank);
    }
};

class SampleRepository
{
    //TODO le déplacer dans mainComponent
public:
    SampleInfos& getSampleInfos(int id)
    {
        return rows[id];
    }

    void createSample(const juce::File& file)
    {
        //TODO il faudrait checker si le reader peut l'ouvrir ptn
        if (file.existsAsFile()) {
            addSample({file, kick, 0});
        }
    }

    void addSample(SampleInfos&& sample)
    {
        if(existsInRepository(sample))
            return;
        ids.push_back(numRows);
        rows.push_back(std::move(sample));
        numRows++;
    }

    void removeSamples(const juce::SparseSet<int>& rowIndexes)
    {
        auto numRowsToDelete = rowIndexes.size();

        std::vector<int> sampleIndexes;
        for(auto i = 0; i < numRowsToDelete ; ++i)
        {
            sampleIndexes.push_back(ids[rowIndexes[i]]);
        }

        //remove the samples
        for(auto i = 0; i < numRowsToDelete; ++i)
        {
            auto index_to_erase = sampleIndexes[i] - i;
            rows.erase(rows.begin() + index_to_erase);
        }

        numRows = numRows - numRowsToDelete;
        ids = std::vector<int>(numRows);
        std::ranges::generate(ids, [n = 0] () mutable { return n++; });
        sortByColumn(currentSortingColumn);
    }


    [[nodiscard]] int getNumRows() const
    { return numRows; }

    juce::String getFieldAsString(Column column, int row)
    {
        auto id = ids[row];
        juce::String text;
        switch (column) {
            case Column::id : text = std::to_string(id);
                break;
            case Column::path : text = rows[id].getPath();
                break;
            case Column::type : text = typeToString(rows[id].type);
                break;
            case Column::name : text = rows[id].getName();
                break;
            case Column::rank : text = juce::String(rows[id].rank);
                break;
            default: break;
        }
        return text;
    }

    static Column idToColumn(int columnId)
    {
        switch (columnId) {
            case 1 : return Column::id;
            case 2 : return Column::path;
            case 3 : return Column::name;
            case 4 : return Column::type;
            case 5 : return Column::rank;
            default:
                jassertfalse; return Column::id;
                    }
                }

    void sortByColumnId(int columnId)
    {
        auto column = idToColumn(columnId);
        sortByColumn(column);
    }

    void sortByColumn(Column columnToSortBy)
    {
        currentSortingColumn = columnToSortBy;

        switch (columnToSortBy) {
            case id:
                std::ranges::sort(ids);
                break;
            case path:
                std::ranges::sort(ids, [this](int a, int b) {
                    return rows[a].getPath().compareNatural(rows[a].getPath());
                });
                break;
            case name:
                std::ranges::sort(ids, [this](int a, int b) {
                    return rows[a].getName().compareNatural(rows[a].getName());
                });
                break;
            case type:
                std::ranges::sort(ids, [this](int a, int b) {
                    return rows[a].type > rows[b].type;
                });
                break;
            case rank:
                std::ranges::sort(ids, [this](int a, int b) {
                    return rows[a].rank > rows[b].rank;
                });
                break;
        }

    }

    void setSampleType(int row, SampleType type)
    {
        rows[ids[row]].type = type;
    }

    SampleType getSampleType(int row)
    {
        return rows[ids[row]].type;
    }

    juce::String serialize()
    {
        juce::String out;
        for(const auto& row : rows){
            out<<row.serialize()<<juce::newLine;
        }
        return out;
    }

private:
    std::vector<int> ids;
    std::vector<SampleInfos> rows;

    int numRows = 0;
    Column currentSortingColumn;

    bool existsInRepository(const SampleInfos& sampleToTest)
    {
        return std::ranges::any_of(rows, [&](auto& i){ return i == sampleToTest; });
    }

    static juce::String typeToString(SampleType type){
        switch(type){
            case SampleType::hats : return "hats";
            case SampleType::kick : return "kick";
            case SampleType::snare : return "snare";
        }
    }
};
