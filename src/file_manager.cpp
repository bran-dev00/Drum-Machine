#include "file_manager.hpp"

FileManager::FileManager(std::filesystem::path samples_root_dir) : samples_root_dir_(samples_root_dir) {};

void FileManager::startCopyQueue(std::set<path_pair_t> file_paths)
{
    successful_copies_.clear();
    copy_errors_.clear();
    has_conflict_ = false;
    is_copying_in_progress_ = true;

    // Clear the Queue if not already empty
    while (!copy_queue_.empty())
    {
        copy_queue_.pop();
    }

    for (const auto &file : file_paths)
    {
        copy_queue_.push(file);
    }

    processNextCopy();
}

void FileManager::processNextCopy()
{
    if (copy_queue_.empty())
    {
        is_copying_in_progress_ = false;
        return;
    }

    current_copying_file_ = copy_queue_.front();
    current_conflict_file_.clear();
    has_conflict_ = false;

    auto dest_path = samples_root_dir_ / current_copying_file_.second;

    // std::cout << dest_path << "\n";

    std::error_code ec;

    // Create subdirectories if needed
    auto dest_parent = dest_path.parent_path();
    if (!std::filesystem::exists(dest_parent, ec))
    {
        // std::cout << "dir to create: " << dest_parent << "\n";

        std::filesystem::create_directories(dest_parent, ec);
        if (ec)
        {
            copy_errors_.emplace_back(current_copying_file_.first.string(), "Failed to create directory: " + ec.message());
            copy_queue_.pop();
            processNextCopy();
            return;
        }
    }

    // File already Exists mark it as conflict path
    if (std::filesystem::exists(dest_path, ec))
    {
        current_conflict_file_ = dest_path;
        has_conflict_ = true;
        return;
    }

    std::filesystem::copy_file(current_copying_file_.first, dest_path, ec);

    if (!ec)
    {
        successful_copies_.push_back(dest_path.string());
    }
    else
    {
        copy_errors_.emplace_back(current_copying_file_.first.string(), ec.message());
    }

    copy_queue_.pop();
    processNextCopy();
}

void FileManager::finishCopy()
{
    is_copying_in_progress_ = false;
    has_conflict_ = false;
    current_copying_file_ = {};
    current_conflict_file_.clear();

    while (!copy_queue_.empty())
    {
        copy_queue_.pop();
    }
}

void FileManager::replaceCurrentFile()
{
    if (copy_queue_.empty())
    {
        return;
    }

    auto dest_path = samples_root_dir_ / current_copying_file_.second;
    std::error_code ec;

    // Delete the file, because file is locked
    if (std::filesystem::exists(dest_path, ec))
    {
        std::filesystem::remove(dest_path, ec);
        if (ec)
        {
            copy_errors_.emplace_back(current_copying_file_.first.string(), "Failed to remove existing file: " + ec.message());
            has_conflict_ = false;
            current_conflict_file_.clear();
            copy_queue_.pop();
            processNextCopy();
            return;
        }
    }

    std::filesystem::copy_file(current_copying_file_.first, dest_path, ec);

    if (!ec)
    {
        successful_copies_.push_back(dest_path.string());
    }
    else
    {
        copy_errors_.emplace_back(current_copying_file_.first.string(), ec.message());
    }

    has_conflict_ = false;
    current_conflict_file_.clear();
    copy_queue_.pop();
    processNextCopy();
}

void FileManager::skipCurrentFile()
{
    if (!copy_queue_.empty())
    {
        copy_queue_.pop();
    }

    has_conflict_ = false;
    current_conflict_file_.clear();
    processNextCopy();
}

void FileManager::renameAndCopyCurrentFile(std::string new_name)
{
    if (copy_queue_.empty())
    {
        return;
    }

    if (new_name.empty())
    {
        copy_errors_.emplace_back(current_copying_file_.first.string(), "Empty filename");
        copy_queue_.pop();
        processNextCopy();
        return;
    }

    std::filesystem::path new_path = samples_root_dir_ / current_copying_file_.second.parent_path() / (new_name + current_copying_file_.first.extension().string());

    if (willConflict(new_path))
    {
        copy_errors_.emplace_back(current_copying_file_.first.string(), "Renamed file also conflicts");
        copy_queue_.pop();
        processNextCopy();
        return;
    }

    std::error_code ec;
    std::filesystem::copy_file(current_copying_file_.first, new_path, ec);

    if (!ec)
    {
        successful_copies_.push_back(new_path.string());
    }
    else
    {
        copy_errors_.emplace_back(current_copying_file_.first.string(), ec.message());
    }

    copy_queue_.pop();
    has_conflict_ = false;
    current_conflict_file_.clear();
    processNextCopy();
}

// Checks to see if there is already a file with the same name in the samples_dir or wherever the destination path is.
bool FileManager::willConflict(const std::filesystem::path &dest_path)
{
    std::error_code ec;
    return std::filesystem::exists(dest_path, ec);
}

// For UI Rendering
std::string FileManager::getCurrentCopyingFilename()
{
    return current_copying_file_.first.filename().string();
}

std::string FileManager::getCurrentConflictFilename()
{
    return current_conflict_file_.filename().string();
}

bool FileManager::hasCopyConflict()
{
    return has_conflict_;
}

bool FileManager::isCopyingInProgress()
{
    return is_copying_in_progress_;
}

std::vector<std::string> FileManager::getSuccessfulCopies()
{
    return successful_copies_;
}

std::vector<std::pair<std::string, std::string>> FileManager::getCopyErrors()
{
    return copy_errors_;
}

int FileManager::getCopyQueueRemaining()
{
    return static_cast<int>(copy_queue_.size());
}

void FileManager::setSamplesPath(std::filesystem::path new_path)
{
    samples_root_dir_ = new_path;
}
