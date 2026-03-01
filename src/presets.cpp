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

    std::string preset_string = "";
    preset_string += "PRESET_NAME:" + preset.getPresetName() + "\n";
    preset_string += "DRUM_PACK_IDX:" + std::to_string(preset.getPresetDrumPack()) + "\n";
    preset_string += "BPM:" + std::to_string(preset.getPresetBpm()) + "\n";

    preset_string += "SEQUENCE_START\n";
    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        Track_t track = preset.getPresetTracks().at(i);
        for (size_t j = 0; j < MAX_STEPS; j++)
        {
            preset_string += track.at(j) ? "1" : "0";
        }
        preset_string += "\n";
    }
    preset_string += "SEQUENCE_END\n";

    preset_string += "VOLUMES_START\n";
    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        preset_string += std::to_string(preset.getPresetTrackVolumes().at(i)) + "\n";
    }
    preset_string += "VOLUMES_END\n";
    preset_string += "PRESET_END\n";

    return preset_string;
}

void Preset::savePresetToFile(Preset preset, std::string file_path)
{
    std::ofstream preset_file(file_path);
    if (preset_file.is_open())
    {
        preset_file << presetToString(preset);
        preset_file.close();
    }
    else
    {
        std::cout << "Unable to open file to save preset!\n";
    }
}

Preset Preset::parsePresetFromFile(std::string file_path)
{
    std::ifstream preset_file(file_path);

    if (preset_file.is_open())
    {
        std::string line;
        // dummy preset
        Preset preset("", 0, {}, 90, {});
        std::getline(preset_file, line);

        if (line.rfind("PRESET_NAME:", 0) == 0)
        {
            std::string preset_name = line.substr(std::string("PRESET_NAME:").length());
            preset.setPresetName(preset_name);
        }
        else
        {
            throw std::runtime_error("Invalid preset file format! Expected PRESET_NAME at the beginning.");
        }

        std::getline(preset_file, line);
        if (line.rfind("DRUM_PACK_IDX:", 0) == 0)
        {
            int drum_pack_idx = std::stoi(line.substr(std::string("DRUM_PACK_IDX:").length()));
            preset.setPresetDrumPack(drum_pack_idx);
        }
        else
        {
            throw std::runtime_error("Invalid preset file format! Expected DRUM_PACK_IDX after PRESET_NAME.");
        }

        std::getline(preset_file, line);
        if (line.rfind("BPM:", 0) == 0)
        {
            int bpm = std::stoi(line.substr(std::string("BPM:").length()));
            preset.setPresetBpm(bpm);
        }
        else
        {
            throw std::runtime_error("Invalid preset file format! Expected BPM after DRUM_PACK_IDX.");
        }

        std::getline(preset_file, line);
        if (line != "SEQUENCE_START")
        {
            throw std::runtime_error("Invalid preset file format! Expected SEQUENCE_START after BPM.");
        }

        std::array<Track_t, NUM_TRACKS> tracks;
        for (size_t i = 0; i < NUM_TRACKS; i++)
        {
            std::getline(preset_file, line);
            tracks.at(i) = parseStringPattern(line);
        }
        preset.setPresetTracks(tracks);
        std::getline(preset_file, line);
        if (line != "SEQUENCE_END")
        {
            throw std::runtime_error("Invalid preset file format! Expected SEQUENCE_END after track patterns.");
        }

        std::getline(preset_file, line);
        if (line != "VOLUMES_START")
        {
            throw std::runtime_error("Invalid preset file format! Expected VOLUMES_START after SEQUENCE_END.");
        }

        std::array<float, NUM_TRACKS> track_volumes;
        for (size_t i = 0; i < NUM_TRACKS; i++)
        {
            std::getline(preset_file, line);
            track_volumes.at(i) = std::stof(line);
        }
        preset.setPresetTrackVolumes(track_volumes);

        std::getline(preset_file, line);
        if (line != "VOLUMES_END")
        {
            throw std::runtime_error("Invalid preset file format! Expected VOLUMES_END after track volumes.");
        }

        std::getline(preset_file, line);
        if (line != "PRESET_END")
        {
            throw std::runtime_error("Invalid preset file format! Expected PRESET_END at the end of the file.");
        }

        return preset;
    }
    else
    {
        std::cout << "Unable to open preset file!\n";
        throw std::runtime_error("Unable to open preset file!");
    }
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
