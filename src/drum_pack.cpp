#include "drum_pack.hpp"

void to_json(json &j, const DrumPack &drum_pack)
{
    j = json::object();
    j["id"] = drum_pack.id;
    j["name"] = drum_pack.name;

    j["samples"] = json::array();
    for (const auto &p : drum_pack.samples)
    {
        j["samples"].push_back(p.string());
    }
}

void from_json(const json &j, DrumPack &drum_pack)
{
    j.at("id").get_to(drum_pack.id);
    j.at("name").get_to(drum_pack.name);

    const auto &samples_j = j.at("samples");
    for (size_t i = 0; i < drum_pack.samples.size() && i < samples_j.size(); i++)
    {
        drum_pack.samples[i] = std::filesystem::path(samples_j.at(i).get<std::string>());
    }
}

