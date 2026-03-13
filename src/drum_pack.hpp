#pragma once

#include <string>
#include <array>
#include <filesystem>

#include <nlohmann/json.hpp>

#include "drum_types.hpp"

using json = nlohmann::json;

struct DrumPack
{
    int id;
    std::string name;
    std::array<std::filesystem::path, NUM_TRACKS> samples;
};

// nlohmann::json hooks for DrumPack
void to_json(json &j, const DrumPack &drum_pack);
void from_json(const json &j, DrumPack &drum_pack);

