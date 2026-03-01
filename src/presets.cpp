#include "presets.hpp"

Track_t Preset::parseStringPattern(std::string track_pattern)
{
    Track_t track;

    std::string clean_string = track_pattern;

    for (size_t i = 0; i < clean_string.size(); i++)
    {
        if (clean_string.at(i) == ' ')
        {
            clean_string.erase(i, 1);
            i--;
        }
    }

    for (size_t i = 0; i < MAX_STEPS; i++)
    {
        if (clean_string.at(i) == '0')
        {
            track.at(i) = false;
        }
        else if (clean_string.at(i) == '1')
        {
            track.at(i) = true;
        }
    }
    return track;
}

std::string Preset::presetToString(Preset preset)
{

    std::string preset_string = preset.getPresetName() + "\n";
    preset_string += "PRESET_START\n";
    preset_string += std::to_string(preset.getPresetDrumPack()) + "\n";
    preset_string += std::to_string(preset.getPresetBpm()) + "\n";

    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        Track_t track = preset.getPresetTracks().at(i);
        for (size_t j = 0; j < MAX_STEPS; j++)
        {
            preset_string += track.at(j) ? "1" : "0";
        }
        preset_string += "\n";
    }

    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        preset_string += std::to_string(preset.getPresetTrackVolumes().at(i)) + "\n";
    }
    preset_string += "PRESET_END\n";

    return preset_string;
}

Preset::Preset(std::string preset_name, int drum_pack_idx, std::array<Track_t, NUM_TRACKS> tracks, int bpm, std::array<float, NUM_TRACKS> track_volumes) : preset_name_(preset_name), drum_pack_idx_(drum_pack_idx), tracks_(tracks), bpm_(bpm), track_volumes_(track_volumes)
{
}

Preset::~Preset() = default;

void Preset::setPresetBpm(int bpm)
{
    bpm_ = bpm;
}

void Preset::setPresetDrumPack(int idx)
{
    drum_pack_idx_ = idx;
}

void Preset::setPresetTrackVolumes(std::array<float, NUM_TRACKS> track_volumes)
{
    track_volumes_ = track_volumes;
}

void Preset::setPresetName(std::string preset_name)
{
    preset_name_ = preset_name;
}

void Preset::setPresetTracks(std::array<Track_t, NUM_TRACKS> tracks)
{
    tracks_ = tracks;
}

std::string Preset::getPresetName()
{
    return preset_name_;
}

int Preset::getPresetDrumPack()
{
    return drum_pack_idx_;
}

std::array<float, NUM_TRACKS> Preset::getPresetTrackVolumes()
{
    return track_volumes_;
}

std::array<Track_t, NUM_TRACKS> Preset::getPresetTracks()
{
    return tracks_;
}

int Preset::getPresetBpm()
{
    return bpm_;
}
