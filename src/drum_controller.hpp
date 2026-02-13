#pragma once

#include <filesystem>
#include <chrono>
#include <string>
#include <array>
#include <vector>
#include "../external/miniaudio/miniaudio.h"
#include "drum_track_model.hpp"

#define MAX_STEPS 16

using namespace std::chrono_literals;
using Track_t = std::array<bool, MAX_STEPS>;


class DrumController
{
private:
    std::array<DrumTrackModel, 2> tracks_;

    bool isPlaying_;
    int bpm_;
    int beatCounter_;
    std::chrono::time_point<std::chrono::steady_clock> lastStep_;
    std::string samplePath_;

    // std::vector<ma_sound> samples_;
    std::vector<std::string> samples_;
    ma_engine engine_;
    

public:
    DrumController();
    ~DrumController();

    void initSequencer();
    void loadInitialSamples();

    void setSequencerNoteTrue(Track_t &track, int index);
    void setSequencerNoteFalse(Track_t &track, int index);

    void resetSequencer(Track_t &track);
    void resetAllTracks();

    std::array<bool, MAX_STEPS> &getSequencerArray();

    std::array<DrumTrackModel, 2> &getTracks();
    DrumTrackModel &getTrackByIndex(int index);

    int &getBeatCounter();
    bool getIsPlaying();

    void step();
    void setBpm(int bpm);
    int getBpm();

    void playSound(std::string &samplePath);
    void playSequencer();
    void pauseSequencer();
    void toggleSequencer();
};
