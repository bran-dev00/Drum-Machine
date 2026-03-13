#include <filesystem>
#include <array>
#include <fstream>
#include <iostream>

#include "drum_pack.hpp"

class DrumPackManager
{
private:
    DrumPack drum_pack_;
    json json_;

    std::filesystem::path root_samples_path_;
    std::filesystem::path save_data_path_;

public:
    // Creation
    // Deletion
    // Edit
    // Scan drum-kits directory
    // Error Handling

    DrumPack createDrumPack(int id, std::string name, std::array<std::filesystem::path, NUM_TRACKS> samples);

    void saveDrumPack(DrumPack &drum_pack);

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