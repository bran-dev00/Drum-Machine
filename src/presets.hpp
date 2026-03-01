#pragma once

#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include "drum_types.hpp"

class Preset
{
private:
    std::string preset_name_;
    int drum_pack_idx_;
    std::array<Track_t, NUM_TRACKS> tracks_;
    int bpm_;
    std::array<float, NUM_TRACKS> track_volumes_;

public:
    Preset(std::string preset_name, int drum_pack_idx, std::array<Track_t, NUM_TRACKS> tracks, int bpm, std::array<float, NUM_TRACKS> track_volumes);
    ~Preset();

    static Track_t parseStringPattern(std::string track_pattern);
    static std::string presetToString(Preset preset);
    static Preset parsePresetFromFile(std::string file_path);

    static void savePresetToFile(Preset preset, std::string file_path);
    static void deletePresetFile(std::string file_path);

    void setPresetBpm(int bpm);
    void setPresetDrumPack(int idx);
    void setPresetTrackVolumes(std::array<float, NUM_TRACKS> track_volumes);
    void setPresetName(std::string preset_name);
    void setPresetTracks(std::array<Track_t, NUM_TRACKS> tracks);

    std::string getPresetName();
    int getPresetDrumPack();
    std::array<float, NUM_TRACKS> getPresetTrackVolumes();
    std::array<Track_t, NUM_TRACKS> getPresetTracks();
    int getPresetBpm();
};
