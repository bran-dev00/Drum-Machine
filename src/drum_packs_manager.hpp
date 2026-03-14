#include <filesystem>
#include <array>
#include <fstream>
#include <iostream>
#include <vector>

#include "drum_pack.hpp"

class DrumPackManager
{
private:
    std::filesystem::path root_samples_path_;
    std::filesystem::path save_data_path_;

public:
    DrumPack createDrumPack(int id, std::string name, std::array<std::filesystem::path, NUM_TRACKS> samples);

    void saveDrumPack(const DrumPack &drum_pack);

    // Load all drum packs from JSON files in save_data_path_
    std::vector<DrumPack> loadDrumPacks() const;

    // Configuration
    void updateRootSamplesPath(std::filesystem::path new_path)
    {
        root_samples_path_ = new_path;
    }

    void updateSaveDataPath(std::filesystem::path new_path)
    {
        save_data_path_ = new_path;
    }

    DrumPackManager() = default;
    DrumPackManager(std::filesystem::path root_samples_path, std::filesystem::path save_data_path);
    ~DrumPackManager() = default;
};