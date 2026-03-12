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

void Preset::deletePresetFile(std::string file_path)
{
    if (std::remove(file_path.c_str()) != 0)
    {
        std::cout << "Error deleting preset file: " << file_path << "\n";
    }
}

void Preset::savePresetJsonToFile(Preset &preset, const std::string &file_path)
{
    std::ofstream file(file_path);

    if (file.is_open())
    {
        json j = preset;
        file << j.dump(4);
    }
}

Preset Preset::loadPresetFromFile(std::string file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open preset file: " + file_path);
    }

    json j;
    file >> j;

    Preset preset;
    from_json(j, preset);
    return preset;
}

// configure parsing to and from json
void to_json(json &j, const Preset preset)
{
    j =
        {
            {"name", preset.preset_name_},
            {"drum_pack_idx", preset.drum_pack_idx_},
            {"tracks", preset.tracks_},
            {"bpm", preset.bpm_},
            {"volumes", preset.track_volumes_},
        };
}

void from_json(const json &j, Preset &p)
{
    j.at("name").get_to(p.preset_name_);
    j.at("drum_pack_idx").get_to(p.drum_pack_idx_);
    j.at("bpm").get_to(p.bpm_);
    j.at("tracks").get_to(p.tracks_);
    j.at("volumes").get_to(p.track_volumes_);
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
