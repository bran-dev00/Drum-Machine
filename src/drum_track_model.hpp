#pragma once

#include <string>
#include <vector>
#include <array>

#define MAX_STEPS 16

using Track_t = std::array<bool,MAX_STEPS>;

class DrumTrackModel 
{
private:
    std::string samplePath_;
    std::string name_;
    Track_t sequencer_;

public:
    DrumTrackModel(const std::string &name, const std::string &samplePath);
    DrumTrackModel();
    ~DrumTrackModel();
    
    std::string getName();
    std::string getSample();
    Track_t &getTrackSequencer();
    
    void setName(std::string &newName);
    void setSample(std::string &samplePath);

};
