#pragma once

#include <string>
#include <vector>
#include <array>
#include <filesystem>

#define MAX_STEPS 16

using Track_t = std::array<bool,MAX_STEPS>;

class DrumTrackModel 
{
private:
    std::filesystem::path samplePath_;
    std::string name_;
    Track_t sequencer_;

public:
    DrumTrackModel(const std::string &name, const std::filesystem::path &samplePath);
    DrumTrackModel();
    ~DrumTrackModel();
    
    std::string getName();
    std::filesystem::path getSample();
    Track_t &getTrackSequencer();
    
    void setName(std::string &newName);
    void setSample(std::filesystem::path &samplePath);

};
