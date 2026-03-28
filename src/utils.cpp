#include "utils.hpp"
#include <algorithm>
#include <iostream>

namespace
{
    // TODO:MP3 and Wav for now, Test Other Audio Extensions
    const std::set<std::string> valid_extensions = {".wav", ".mp3"};
}

bool FileValidation::isValidAudioExtension(const std::string &extension)
{
    std::string ext_lower = extension;
    std::transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(), ::tolower);
    return valid_extensions.find(ext_lower) != valid_extensions.end();
}

std::vector<std::filesystem::path> FileValidation::validateDirectory(const std::filesystem::path &dir_path)
{
    std::vector<std::filesystem::path> validated_files;

    if (!std::filesystem::is_directory(dir_path))
    {
        return validated_files;
    }

    // Only copy over valid audio files, ignore the rest.
    for (const auto &entry : std::filesystem::recursive_directory_iterator(dir_path))
    {
        if (entry.is_regular_file())
        {
            if (entry.path().has_extension())
            {
                std::string ext = entry.path().extension().string();
                if (isValidAudioExtension(ext))
                {
                    validated_files.push_back(entry.path());
                }
            }
        }
    }

    return validated_files;
}

std::vector<path_pair_t> FileValidation::getFilesWithRelativePaths(const std::filesystem::path &dir_path)
{
    std::vector<path_pair_t> result;

    if (!std::filesystem::is_directory(dir_path))
    {
        return result;
    }

    for (const auto &entry : std::filesystem::recursive_directory_iterator(dir_path))
    {
        if (entry.is_regular_file())
        {
            if (entry.path().has_extension())
            {
                std::string ext = entry.path().extension().string();
                if (isValidAudioExtension(ext))
                {
                    std::filesystem::path relative_path = PathUtils::computeRelativePath(entry.path(), dir_path);
                    result.push_back({entry.path(), relative_path});
                }
            }
        }
    }

    return result;
}

std::vector<std::filesystem::path> FileValidation::validateFile(const std::filesystem::path &file_path)
{
    std::vector<std::filesystem::path> result;

    if (!std::filesystem::is_regular_file(file_path))
    {
        return result;
    }

    if (file_path.has_extension())
    {
        std::string ext = file_path.extension().string();
        if (isValidAudioExtension(ext))
        {
            result.push_back(file_path);
        }
    }

    return result;
}

FileUtils::SampleDirectoryStructure FileUtils::getSamplesDirectoryStructure(const std::filesystem::path &root)
{
    SampleDirectoryStructure result;

    if (!std::filesystem::is_directory(root))
    {
        return result;
    }

    for (const auto &entry : std::filesystem::directory_iterator(root))
    {
        if (entry.is_directory())
        {
            SampleFolder folder;
            folder.name = entry.path().filename().string();
            for (const auto &file : std::filesystem::directory_iterator(entry.path()))
            {
                if (file.is_regular_file() && file.path().has_extension())
                {
                    std::string ext = file.path().extension().string();
                    if (FileValidation::isValidAudioExtension(ext))
                    {
                        folder.samples.push_back(file.path());
                    }
                }
            }
            if (!folder.samples.empty())
            {
                result.folders.push_back(std::move(folder));
            }
        }
        else if (entry.is_regular_file() && entry.path().has_extension())
        {
            std::string ext = entry.path().extension().string();
            if (FileValidation::isValidAudioExtension(ext))
            {
                result.root_samples.push_back(entry.path());
            }
        }
    }

    return result;
}
