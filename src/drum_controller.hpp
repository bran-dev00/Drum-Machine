#pragma once

#include <filesystem>
#include <chrono>
#include <string>
#include <array>
#include <vector>
#include "../external/miniaudio/miniaudio.h"
#include "drum_track_model.hpp"

#define MAX_STEPS 16
#define NUM_TRACKS 8

using namespace std::chrono_literals;
using Track_t = std::array<bool, MAX_STEPS>;

class DrumController
{
private:
    std::array<DrumTrackModel, NUM_TRACKS> tracks_;

    const int QUARTER_NOTE_MULT_ = 1;
    const int SIXTEENTH_NOTE_MULT_ = 4;
    const int EIGHT_NOTE_MULT_ = 2;

    bool is_playing_;
    int bpm_;
    int beatCounter_;
    std::string base_assets_dir_;
    std::chrono::time_point<std::chrono::steady_clock> lastStep_;

    std::vector<std::string> samples_paths_;
    std::array<ma_sound *, NUM_TRACKS> sounds_;
    std::array<bool, NUM_TRACKS> sound_initialized_;
    std::array<float, NUM_TRACKS> track_volumes_;

    std::vector<std::string> drum_packs_;
    std::string curr_drum_pack_;

    ma_engine engine_;

    float volume_;

public:
    DrumController();
    ~DrumController();

    void initSequencer();
    void initSoundArray();

    void loadInitialSamples();
    void loadSamples(const std::string sample_path);

    void setSequencerNoteTrue(Track_t &track, int index);
    void setSequencerNoteFalse(Track_t &track, int index);

    void resetSequencer(Track_t &track);
    void resetAllTracks();

    std::string extractSampleName(std::string file_path);
    std::string extractDirName(std::string file_path);

    std::array<bool, MAX_STEPS> &getSequencerArray();

    std::array<DrumTrackModel, NUM_TRACKS> &getTracks();
    DrumTrackModel &getTrackByIndex(int index);

    int &getBeatCounter();
    bool getIsPlaying();

    void step();
    void setBpm(int bpm);
    int getBpm();

    void setDrumPack(int index);
    std::string getCurrDrumPack();
    std::vector<std::string> getDrumPacks();

    // re-scan assets directory for all the drum packs
    void scanDrumPacks();

    void setMasterVolume(float value);
    float getMasterVolume();

    void initTrackVolumesArr();
    ma_sound *getSound(int index);
    float getSoundVolume(int track_idx);
    void setSoundVolume(int track_idx, float value);
    std::array<float, NUM_TRACKS> getTrackVolumes();

    void playSound(int track_idx);
    void playSequencer();
    void pauseSequencer();
    void toggleSequencer();
};
