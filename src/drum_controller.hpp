
#pragma once

#include <filesystem>
#include <chrono>
#include <string>
#include <array>
#include "../external/miniaudio/miniaudio.h"

#define MAX_STEPS 16

using namespace std::chrono_literals;
class DrumController
{
private:
    std::array<bool, MAX_STEPS> sequencerArr{};
    int currStep;
    bool isPlaying_;
    int bpm_;
    int beatCounter_;
    std::chrono::time_point<std::chrono::steady_clock> lastStep;
    std::string samplePath;
    
    ma_engine engine_;

public:
    DrumController(const std::string &samplePath);
    ~DrumController();

    void initSequencer();
    void setSequencerNoteTrue(int index);
    void setSequencerNoteFalse(int index);
    void resetSequencer();

    std::array<bool, MAX_STEPS> &getSequencerArray();
    std::string getSequencerString();

    int &getBeatCounter();
    bool getIsPlaying();

    void step();
    void setBpm(int bpm);

    // void playSound(std::wstring &samplePath);
    void playSound(std::string &samplePath);
    void playSequencer();
    void pauseSequencer();
    void toggleSequencer();
};
