#include "drum_packs_manager.hpp"

DrumPackManager::DrumPackManager(std::filesystem::path root_samples_path, std::filesystem::path save_data_path) : root_samples_path_(root_samples_path), save_data_path_(save_data_path)
{
}

void DrumPackManager::saveDrumPack(DrumPack &drum_pack)
{
    std::ofstream file(save_data_path_);

    if (file.is_open())
    {
        json j = drum_pack;
        file << j.dump(4);
    }
}