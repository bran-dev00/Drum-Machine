#pragma once
#include "utils.hpp"

#include <vector>
#include <filesystem>
#include <queue>
#include <algorithm>

class FileManager
{
private:
    std::filesystem::path samples_root_dir_;
    std::queue<path_pair_t> copy_queue_;
    std::vector<std::string> successful_copies_;
    std::vector<std::pair<std::string, std::string>> copy_errors_;

    path_pair_t current_copying_file_;
    std::filesystem::path current_conflict_file_;

    bool is_copying_in_progress_;
    bool has_conflict_;

public:
    FileManager() = default;
    FileManager(std::filesystem::path samples_root_dir);
    ~FileManager() = default;

    bool willConflict(const std::filesystem::path &dest_path);
    void setSamplesPath(std::filesystem::path new_path);

    void startCopyQueue(std::set<path_pair_t> file_paths);
    void processNextCopy();

    void replaceCurrentFile();
    void skipCurrentFile();
    void renameAndCopyCurrentFile(std::string new_name);
    void finishCopy();

    std::string getCurrentCopyingFilename();
    std::string getCurrentConflictFilename();

    // For UI Rendering
    bool hasCopyConflict();
    bool isCopyingInProgress();
    std::vector<std::string> getSuccessfulCopies();
    std::vector<std::pair<std::string, std::string>> getCopyErrors();
    int getCopyQueueRemaining();
};