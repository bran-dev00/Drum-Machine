#pragma once

#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <filesystem>
#include "drum_types.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Preset
{
private:
    std::string preset_name_;
    int drum_pack_idx_;
    std::array<Track_t, NUM_TRACKS> tracks_;
    int bpm_;
    std::array<float, NUM_TRACKS> track_volumes_;

public:
    Preset() = default;
    Preset(std::string preset_name, int drum_pack_idx, std::array<Track_t, NUM_TRACKS> tracks, int bpm, std::array<float, NUM_TRACKS> track_volumes);
    ~Preset();

    static Track_t parseStringPattern(std::string track_pattern);
    static Preset loadPresetFromFile(std::string file_path);

    static void savePresetJsonToFile(Preset &preset, const std::string &file_path);
    static void deletePresetJsonFile(Preset &preset, const std::string &file_path);

    // TODO:DELETE LATER
    static void deletePresetFile(std::string file_path);

    friend void to_json(json &j, const Preset preset);
    friend void from_json(const json &j, Preset &preset);

    // getters & setters
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
