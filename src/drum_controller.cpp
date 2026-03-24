#include "drum_controller.hpp"
#include <string>
#include <iostream>

DrumController::DrumController()
{
    is_playing_ = false;
    lastStep_ = std::chrono::steady_clock::now();
    beatCounter_ = 0;
    bpm_ = 90;

    drum_packs_save_dir_ = (std::filesystem::current_path() / L"data" / L"drum-kits");
    samples_root_dir_ = (std::filesystem::current_path() / L"assets" / L"samples");

    main_session_file_path_ = (std::filesystem::current_path() / L"data" / L"main_session" / L"main_session.json").string();

    ma_engine_init(NULL, &engine_);
    ma_engine_set_volume(&engine_, .5f);

    drum_pack_manager_ = DrumPackManager(samples_root_dir_, drum_packs_save_dir_);

    is_copying_in_progress_ = false;
    has_conflict_ = false;

    // first initialization
    sound_initialized_.fill(false);
    scanDrumPacks();
    loadInitialSamples();
    initSoundArray();
    initSequencer();
    initTrackVolumesArr();

    scanPresets();
}

DrumController::~DrumController()
{
    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        ma_sound_uninit(sounds_[i]);
        delete sounds_[i];
    }
    ma_engine_uninit(&engine_);
}

Preset DrumController::initBlankPreset()
{
    std::array<Track_t, NUM_TRACKS> preset_tracks;
    std::array<std::string, NUM_TRACKS> demo_preset_tracks = {
        "0000 0000 0000 0000",
        "0000 0000 0000 0000",
        "0000 0000 0000 0000",
        "0000 0000 0000 0000",
        "0000 0000 0000 0000",
        "0000 0000 0000 0000",
        "0000 0000 0000 0000",
        "0000 0000 0000 0000",
    };

    std::array<float, NUM_TRACKS> default_volumes;
    default_volumes.fill(1.0);

    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        preset_tracks.at(i) = Preset::parseStringPattern(demo_preset_tracks.at(i));
    }

    Preset blank_preset = {"BlankPreset", 0, preset_tracks, 90, default_volumes};
    return blank_preset;
}

// Both ma_sound & string names for data_model
void DrumController::loadSamples(const std::string sample_path)
{
    samples_paths_.clear();

    for (const auto &entry : std::filesystem::directory_iterator(sample_path))
    {
        std::string file_name = entry.path().string();
        samples_paths_.push_back(file_name);
    }
}

void DrumController::loadInitialSamples()
{
    if (drum_packs_.empty())
    {
        return;
    }

    curr_drum_pack_index_ = 0;
    samples_paths_.clear();

    const auto &pack = drum_packs_.at(0);
    for (const auto &path : pack.samples)
    {
        if (!path.empty())
        {
            samples_paths_.push_back(path.string());
        }
    }
}

// Copy Conflict Handling

void DrumController::startCopyQueue(std::set<path_pair_t> file_paths)
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

void DrumController::processNextCopy()
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

void DrumController::replaceCurrentFile()
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

void DrumController::skipCurrentFile()
{
    if (!copy_queue_.empty())
    {
        copy_queue_.pop();
    }

    has_conflict_ = false;
    current_conflict_file_.clear();
    processNextCopy();
}

void DrumController::renameAndCopyCurrentFile(std::string new_name)
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
bool DrumController::willConflict(const std::filesystem::path &dest_path)
{
    std::error_code ec;
    return std::filesystem::exists(dest_path, ec);
}

void DrumController::finishCopy()
{
    is_copying_in_progress_ = false;
    has_conflict_ = false;
    current_copying_file_ = {};
    current_conflict_file_.clear();

    while (!copy_queue_.empty())
    {
        copy_queue_.pop();
    }

    scanDrumPacks();
    loadInitialSamples();
    initSequencer();
}

// For UI Rendering
std::string DrumController::getCurrentCopyingFilename()
{
    return current_copying_file_.first.filename().string();
}

std::string DrumController::getCurrentConflictFilename()
{
    return current_conflict_file_.filename().string();
}

bool DrumController::hasCopyConflict()
{
    return has_conflict_;
}

bool DrumController::isCopyingInProgress()
{
    return is_copying_in_progress_;
}

std::vector<std::string> DrumController::getSuccessfulCopies()
{
    return successful_copies_;
}

std::vector<std::pair<std::string, std::string>> DrumController::getCopyErrors()
{
    return copy_errors_;
}

int DrumController::getCopyQueueRemaining()
{
    return static_cast<int>(copy_queue_.size());
}

void DrumController::initSoundArray()
{
    for (size_t i = 0; i < sounds_.size(); i++)
    {
        sounds_[i] = new ma_sound();
    }
}

// Initialize the sequencer with the samples for the current drum-pack
void DrumController::initSequencer()
{

    for (size_t i = 0; i < tracks_.size(); i++)
    {
        std::string track_name;
        // If there is already a sound initialized for this slot, uninitialize it first
        if (sound_initialized_[i] && sounds_.at(i) != nullptr)
        {
            ma_sound_stop(sounds_[i]);
            ma_sound_uninit(sounds_[i]);
            sound_initialized_[i] = false;
        }

        if (i < samples_paths_.size())
        {
            track_name = PathUtils::extractSampleName(samples_paths_[i]);
            tracks_[i] = DrumTrackModel(track_name, samples_paths_[i]);

            if (ma_sound_init_from_file(&engine_, samples_paths_[i].c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, sounds_[i]) == MA_SUCCESS)
            {
                sound_initialized_[i] = true;
            }
            else
            {
                sound_initialized_[i] = false;
            }
        }
        else
        {
            // No sample for this track index, creates an empty track
            tracks_[i] = DrumTrackModel("", "");
            sound_initialized_[i] = false;
        }
    }
}

void DrumController::updateTracks(std::array<Track_t, NUM_TRACKS> tracks)
{
    if (tracks.empty())
    {
        std::cout << "tracks is empty\n";
        return;
    }

    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        Track_t track = tracks.at(i);

        tracks_.at(i).getTrackSequencer() = track;
    }
}

void DrumController::playSound(int track_idx)
{
    if (!sound_initialized_[track_idx])
        return;

    auto sound = sounds_[track_idx];

    ma_sound_seek_to_pcm_frame(sound, 0);
    ma_sound_start(sound);
}

void DrumController::step()
{
    auto now = std::chrono::steady_clock::now();

    std::chrono::duration<double> secondsPerBeat(60.0 / bpm_);
    auto bpmToMs = std::chrono::duration_cast<std::chrono::milliseconds>(secondsPerBeat / SIXTEENTH_NOTE_MULT_);

    if (is_playing_ && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStep_) > bpmToMs)
    {
        for (size_t i = 0; i < NUM_TRACKS; i++)
        {
            auto &track = tracks_[i];
            auto sequencer = track.getTrackSequencer();
            auto sample = track.getSample();

            if (sequencer.at(beatCounter_) == true && !sequencer.empty())
            {
                playSound(i);
            }
        }

        lastStep_ = now;
        beatCounter_ = (beatCounter_ + 1) % MAX_STEPS;
    }
    else
    {
        return;
    }
}

void DrumController::initTrackVolumesArr()
{
    if (!sounds_.empty())
    {
        for (int i = 0; i < NUM_TRACKS; i++)
        {
            ma_sound *pSound = sounds_.at(i);
            ma_sound_set_volume(pSound, 1);
            track_volumes_.at(i) = ma_sound_get_volume(pSound);
        }
    }
}

void DrumController::setMasterVolume(float value = .5f)
{
    if (value < 0)
    {
        volume_ = 0;
    }

    // TODO: Volume Limit
    volume_ = value;
    ma_engine_set_volume(&engine_, value);
}

float DrumController::getMasterVolume()
{
    return ma_engine_get_volume(&engine_);
}

ma_sound *DrumController::getSound(int index)
{
    return sounds_.at(index);
}

void DrumController::setSoundVolume(int track_idx, float value)
{
    ma_sound *pSound = sounds_.at(track_idx);
    ma_sound_set_volume(pSound, value);
    track_volumes_.at(track_idx) = ma_sound_get_volume(pSound);
}

float DrumController::getSoundVolume(int track_idx)
{
    return track_volumes_.at(track_idx);
}

std::array<float, NUM_TRACKS> DrumController::getTrackVolumes()
{
    if (sounds_.empty())
    {
        initTrackVolumesArr();
    }
    return track_volumes_;
}

//---Drum Packs---

void DrumController::scanDrumPacks()
{
    drum_packs_ = drum_pack_manager_.loadDrumPacks();
}

int DrumController::getDrumPackIdx(std::string drum_pack_name)
{
    for (size_t i = 0; i < drum_packs_.size(); i++)
    {
        if (drum_packs_.at(i).name == drum_pack_name)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void DrumController::setDrumPack(int index)
{
    if (index < 0 || index >= static_cast<int>(drum_packs_.size()))
    {
        std::cout << "drum_packs_ index out of bounds!\n";
        return;
    }

    if (curr_drum_pack_index_ == index)
    {
        return;
    }

    is_playing_ = false;
    beatCounter_ = 0;
    curr_drum_pack_index_ = index;

    samples_paths_.clear();
    const auto &pack = drum_packs_.at(index);
    for (const auto &path : pack.samples)
    {
        if (!path.empty())
        {
            samples_paths_.push_back(path.string());
        }
    }

    // update sounds here
    initSequencer();
}

std::string DrumController::getCurrDrumPack()
{
    if (curr_drum_pack_index_ < 0 || curr_drum_pack_index_ >= static_cast<int>(drum_packs_.size()))
    {
        return "";
    }

    return drum_packs_.at(curr_drum_pack_index_).name;
}

std::vector<std::string> DrumController::getDrumPacks()
{
    std::vector<std::string> names;
    names.reserve(drum_packs_.size());

    for (const auto &pack : drum_packs_)
    {
        names.push_back(pack.name);
    }

    return names;
}

void DrumController::loadPreset(Preset preset)
{
    setBpm(preset.getPresetBpm());
    setDrumPack(preset.getPresetDrumPack());
    updateTracks(preset.getPresetTracks());
    is_playing_ = false;
    beatCounter_ = 0;
}

std::vector<Preset> DrumController::getPresetsList()
{
    return presets_list_;
}

// scan presets directory for preset files and update presets_list accordingly
// TODO: handle preset file parsing errors, skip invalid files and keep/load the valid ones
void DrumController::scanPresets()
{
    std::string presets_dir = (std::filesystem::current_path() / L"data" / L"presets").string();

    presets_list_.clear();

    for (const auto &entry : std::filesystem::directory_iterator(presets_dir))
    {
        if (entry.is_regular_file())
        {
            std::string file_path = entry.path().string();
            // Preset preset = Preset::parsePresetFromFile(file_path);
            Preset preset = Preset::loadPresetFromFile(file_path);
            presets_list_.push_back(preset);

            // DEBUG
            /* for (size_t i = 0; i < presets_list_.size(); i++)
            {
                std::cout << "list size: " << presets_list_.size() << "\n";
                std::cout << "Preset " << i << ": " << presets_list_.at(i).getPresetName() << "\n";
            } */
        }
    }
}

// helper function
static std::string toSnakeCase(std::string input_str)
{
    std::string output_str = input_str;

    std::replace(output_str.begin(), output_str.end(), ' ', '_');

    // to lowercase
    std::transform(output_str.begin(), output_str.end(), output_str.begin(),
                   [](unsigned char c)
                   { return static_cast<unsigned char>(std::tolower(c)); });

    return output_str;
}

void DrumController::addPreset(Preset preset)
{
    std::string preset_file_path = (std::filesystem::current_path() / L"data" / "presets" / (toSnakeCase(preset.getPresetName()) + ".json")).string();
    Preset::savePresetJsonToFile(preset, preset_file_path);

    // update presets_list_
    scanPresets();
}

void DrumController::deletePreset(int index)
{

    if (presets_list_.empty())
    {
        return;
    }

    if (index < 0 || index >= presets_list_.size())
    {
        std::cout << "presets_list_ index out of bounds!\n";
        return;
    }

    Preset preset_to_delete = presets_list_.at(index);
    std::string preset_file_path = (std::filesystem::current_path() / L"data" / "presets" / (toSnakeCase(preset_to_delete.getPresetName()) + ".json")).string();
    Preset::deletePresetFile(preset_file_path);

    // update presets_list_
    scanPresets();
}

void DrumController::setBpm(int bpm)
{
    bpm_ = bpm;
}

int DrumController::getBpm()
{
    return bpm_;
}

void DrumController::setSequencerNoteTrue(Track_t &track, int index)
{
    if (index < 0 || index > MAX_STEPS - 1)
    {
        return;
    }
    track[index] = true;
}

void DrumController::setSequencerNoteFalse(Track_t &track, int index)
{
    if (index < 0 || index > MAX_STEPS - 1)
    {
        return;
    }
    track[index] = false;
}

void DrumController::resetSequencer(Track_t &track)
{
    track.fill(false);
}

void DrumController::resetAllTracks()
{

    for (size_t i = 0; i < tracks_.size(); i++)
    {
        tracks_[i].getTrackSequencer().fill(false);
    }
    is_playing_ = false;
    beatCounter_ = 0;
}

int &DrumController::getBeatCounter()
{
    return this->beatCounter_;
}

bool DrumController::getIsPlaying()
{
    return this->is_playing_;
}

std::array<DrumTrackModel, NUM_TRACKS> &DrumController::getTracks()
{
    return tracks_;
}

DrumTrackModel &DrumController::getTrackByIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(tracks_.size()))
    {
        throw "index out of bounds";
    }

    return tracks_.at(index);
}

// Main Session File Saving
void DrumController::saveSession(Preset preset)
{
    std::ofstream session_file(main_session_file_path_);

    if (session_file.is_open())
    {
        Preset::savePresetJsonToFile(preset, main_session_file_path_);
        session_file.close();
    }
    else
    {
        std::cout << "could not open filepath: " << main_session_file_path_ << "\n";
    }
}

void DrumController::loadSession()
{
    std::ifstream session_file(main_session_file_path_);
    if (session_file.is_open())
    {
        Preset preset = Preset::loadPresetFromFile(main_session_file_path_);
        loadPreset(preset);
    }
    else
    {
        // create new main_session file
        std::ofstream session_file(main_session_file_path_);
        if (session_file.is_open())
        {
            Preset preset = initBlankPreset();
            preset.setPresetName("Main_Session");
            Preset::savePresetJsonToFile(preset, main_session_file_path_);
        }
        else
        {
            std::cout << "failed to open/create session-file: " << main_session_file_path_ << "\n";
        }
    }
}

void DrumController::playSequencer()
{
    is_playing_ = true;
}

void DrumController::pauseSequencer()
{
    is_playing_ = false;
}

void DrumController::toggleSequencer()
{
    is_playing_ = !is_playing_;
}
