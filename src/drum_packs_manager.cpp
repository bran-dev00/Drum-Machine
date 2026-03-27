#include "drum_packs_manager.hpp"
#include "utils.hpp"

DrumPackManager::DrumPackManager(std::filesystem::path root_samples_path, std::filesystem::path save_data_path)
    : root_samples_path_(root_samples_path), save_data_path_(save_data_path)
{
}

DrumPack DrumPackManager::createDrumPack(int id, std::string name, std::array<std::filesystem::path, NUM_TRACKS> samples)
{
    DrumPack pack{};
    pack.id = id;
    pack.name = name;
    pack.samples = samples;
    return pack;
}

void DrumPackManager::saveDrumPack(const DrumPack &drum_pack)
{
    if (!std::filesystem::exists(save_data_path_))
    {
        std::filesystem::create_directories(save_data_path_);
    }

    auto file_path = save_data_path_ / (StringUtils::toSnakeCase(drum_pack.name) + ".json");
    std::ofstream file(file_path);

    if (file.is_open())
    {
        json j = drum_pack;
        file << j.dump(4);
    }
}

std::vector<DrumPack> DrumPackManager::loadDrumPacks() const
{
    std::vector<DrumPack> packs;

    if (!std::filesystem::exists(save_data_path_))
    {
        return packs;
    }

    for (const auto &entry : std::filesystem::directory_iterator(save_data_path_))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open())
        {
            continue;
        }

        json j;
        file >> j;
        DrumPack pack = j.get<DrumPack>();
        packs.push_back(pack);
    }

    return packs;
}