#pragma once

#include <filesystem>
#include <chrono>
#include <string>
#include <array>
#include <set>
#include <vector>
#include <utility>
#include <fstream>
#include <queue>

#include "../external/miniaudio/miniaudio.h"
#include "drum_track_model.hpp"
#include "drum_types.hpp"
#include "utils.hpp"

#include "presets.hpp"
#include "drum_packs_manager.hpp"
#include "file_manager.hpp"

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

    std::string main_session_file_path_;

    std::filesystem::path samples_root_dir_;
    std::filesystem::path drum_packs_save_dir_;

    DrumPackManager drum_pack_manager_;
    FileManager file_manager_;

    std::chrono::time_point<std::chrono::steady_clock> lastStep_;

    std::vector<std::string> samples_paths_;
    std::array<ma_sound *, NUM_TRACKS> sounds_;
    std::array<bool, NUM_TRACKS> sound_initialized_;

    std::array<float, NUM_TRACKS> track_volumes_;

    std::vector<DrumPack> drum_packs_;
    int curr_drum_pack_index_ = -1;

    std::vector<Preset> presets_list_;

    ma_engine engine_;

    float volume_;

    // files to be copied
    std::queue<path_pair_t> copy_queue_;
    std::vector<std::string> successful_copies_;
    std::vector<std::pair<std::string, std::string>> copy_errors_;

    path_pair_t current_copying_file_;
    std::filesystem::path current_conflict_file_;

    bool is_copying_in_progress_;
    bool has_conflict_;

    bool willConflict(const std::filesystem::path &dest_path);

public:
    DrumController();
    ~DrumController();

    void initSequencer();
    void initSoundArray();
    Preset initBlankPreset();

    void loadInitialSamples();
    void loadSamples(const std::string sample_path);

    void startCopyQueue(std::set<path_pair_t> file_paths);
    void processNextCopy();

    void replaceCurrentFile();
    void skipCurrentFile();
    void renameAndCopyCurrentFile(std::string new_name);
    void finishCopy();

    std::string getCurrentCopyingFilename();
    std::string getCurrentConflictFilename();

    // For UI Rendering
    bool hasCopyConflict();
    bool isCopyingInProgress();
    std::vector<std::string> getSuccessfulCopies();
    std::vector<std::pair<std::string, std::string>> getCopyErrors();
    int getCopyQueueRemaining();

    void setSequencerNoteTrue(Track_t &track, int index);
    void setSequencerNoteFalse(Track_t &track, int index);

    void updateTracks(std::array<Track_t, NUM_TRACKS> tracks);
    void resetSequencer(Track_t &track);
    void resetAllTracks();

    std::array<DrumTrackModel, NUM_TRACKS> &getTracks();
    DrumTrackModel &getTrackByIndex(int index);

    int &getBeatCounter();
    bool getIsPlaying();

    void step();
    void setBpm(int bpm);
    int getBpm();

    // File
    void saveSession(Preset preset);
    void loadSession();

    // Drum Packs
    // re-scan drum pack JSON directory for all the drum packs
    void scanDrumPacks();
    void setDrumPack(int index);
    std::string getCurrDrumPack();
    std::vector<std::string> getDrumPacks();
    int getDrumPackIdx(std::string drum_pack_name);
    void addDrumPack(std::string name, std::array<std::filesystem::path, NUM_TRACKS> samples);
    void deleteDrumPack(int index);
    std::filesystem::path getSamplesRootDir();

    // Presets
    void scanPresets();
    void addPreset(Preset preset);
    std::vector<Preset> getPresetsList();
    void loadPreset(Preset preset);
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
