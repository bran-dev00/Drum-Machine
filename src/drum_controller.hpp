#pragma once

#include <filesystem>
#include <chrono>
#include <string>
#include <array>
#include "../external/miniaudio/miniaudio.h"

#define MAX_STEPS 16

using namespace std::chrono_literals;
using Track_t = std::array<bool, MAX_STEPS>;

class DrumController
{
private:
    std::array<bool, MAX_STEPS> sequencerArr{};

    std::array<Track_t, 2> tracks_{};
    bool isPlaying_;
    int bpm_;
    int beatCounter_;
    std::chrono::time_point<std::chrono::steady_clock> lastStep_;
    std::string samplePath_;

    ma_engine engine_;

public:
    DrumController(const std::string &samplePath);
    ~DrumController();

    void initSequencer();
    void setSequencerNoteTrue(Track_t &track, int index);
    void setSequencerNoteFalse(Track_t &track, int index);

    void resetSequencer(Track_t &track);
    void resetAllTracks();

    std::array<bool, MAX_STEPS> &getSequencerArray();

    std::array<Track_t, 2> &getTracks();
    Track_t &getTrackByIndex(int index);

    std::string getSequencerString();

    int &getBeatCounter();
    bool getIsPlaying();

    void step();
    void setBpm(int bpm);

    void playSound(std::string &samplePath);
    void playSequencer();
    void pauseSequencer();
    void toggleSequencer();
};
