#pragma once
#include <filesystem>
#include <vector>
#include <set>
#include <string>

using path_pair_t = std::pair<std::filesystem::path, std::filesystem::path>;

namespace FileValidation
{
    std::vector<std::filesystem::path> validateDirectory(const std::filesystem::path &dir_path);
    std::vector<path_pair_t> getFilesWithRelativePaths(const std::filesystem::path &dir_path);
    std::vector<std::filesystem::path> validateFile(const std::filesystem::path &file_path);
    bool isValidAudioExtension(const std::string &extension);
}

namespace PathUtils
{
    inline std::string extractSampleName(const std::filesystem::path &path)
    {
        return path.stem().string();
    }

    inline std::string extractDirName(const std::filesystem::path &path)
    {
        return path.filename().string();
    }

    // For handling directories
    inline std::filesystem::path computeRelativePath(const std::filesystem::path &full_path, const std::filesystem::path &base_dir)
    {
        std::filesystem::path base_name = base_dir.filename();
        std::filesystem::path relative_from_base = std::filesystem::relative(full_path, base_dir);
        return base_name / relative_from_base;
    }
}