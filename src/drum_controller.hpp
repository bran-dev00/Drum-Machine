#pragma once

#include <filesystem>
#include <chrono>
#include <string>
#include <array>
#include <vector>
#include <utility>
#include <fstream>

#include "../external/miniaudio/miniaudio.h"
#include "drum_track_model.hpp"
#include "drum_types.hpp"

#include "presets.hpp"

using namespace std::chrono_literals;

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
    std::string drum_kit_assets_path_;
    std::string main_session_file_path_;

    std::chrono::time_point<std::chrono::steady_clock> lastStep_;

    std::vector<std::string> samples_paths_;
    std::array<ma_sound *, NUM_TRACKS> sounds_;
    std::array<bool, NUM_TRACKS> sound_initialized_;

    std::array<float, NUM_TRACKS> track_volumes_;

    std::vector<std::string> drum_packs_;
    std::string curr_drum_pack_;

    std::vector<Preset> presets_list_;

    ma_engine engine_;

    float volume_;

public:
    DrumController();
    ~DrumController();

    void initSequencer();
    void initSoundArray();
    void initDemoPreset();
    void loadInitialSamples();
    void loadSamples(const std::string sample_path);

    void setSequencerNoteTrue(Track_t &track, int index);
    void setSequencerNoteFalse(Track_t &track, int index);

    void updateTracks(std::array<Track_t, NUM_TRACKS> tracks);
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

    // File
    void saveSession();
    void loadSession();

    // Drum Packs
    // re-scan assets directory for all the drum packs
    void scanDrumPacks();
    void setDrumPack(int index);
    std::string getCurrDrumPack();
    std::vector<std::string> getDrumPacks();
    int getDrumPackIdx(std::string drum_pack_path);

    // Presets
    void scanPresets();
    void addPreset(Preset preset);
    std::vector<Preset> getPresetsList();
    void loadPreset(int index);
    void deletePreset(int index);

    // Volume & Sounds
    void setMasterVolume(float value);
    float getMasterVolume();
    void initTrackVolumesArr();

    ma_sound *getSound(int index);
    float getSoundVolume(int track_idx);
    void setSoundVolume(int track_idx, float value);
    std::array<float, NUM_TRACKS> getTrackVolumes();

    // Playback Controls
    void playSound(int track_idx);
    void playSequencer();
    void pauseSequencer();
    void toggleSequencer();
};
