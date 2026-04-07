#include "drum_controller.hpp"
#include "utils.hpp"
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
    file_manager_ = FileManager(samples_root_dir_);

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
        samples_paths_.push_back(entry.path());
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
            samples_paths_.push_back(path);
        }
    }
}

// Copy Conflict Handling

void DrumController::startCopyQueue(std::set<path_pair_t> file_paths)
{
    file_manager_.startCopyQueue(file_paths);
}

void DrumController::processNextCopy()
{
    file_manager_.processNextCopy();
}

void DrumController::replaceCurrentFile()
{
    file_manager_.replaceCurrentFile();
}

void DrumController::skipCurrentFile()
{
    file_manager_.skipCurrentFile();
}

void DrumController::renameAndCopyCurrentFile(std::string new_name)
{
    file_manager_.renameAndCopyCurrentFile(new_name);
}

// Checks to see if there is already a file with the same name in the samples_dir or wherever the destination path is.
bool DrumController::willConflict(const std::filesystem::path &dest_path)
{
    return file_manager_.willConflict(dest_path);
}

void DrumController::finishCopy()
{
    file_manager_.finishCopy();

    scanDrumPacks();
    loadInitialSamples();
    initSequencer();
}

// For UI Rendering
std::string DrumController::getCurrentCopyingFilename()
{
    return file_manager_.getCurrentCopyingFilename();
}

std::string DrumController::getCurrentConflictFilename()
{
    return file_manager_.getCurrentConflictFilename();
}

bool DrumController::hasCopyConflict()
{
    return file_manager_.hasCopyConflict();
}

bool DrumController::isCopyingInProgress()
{
    return file_manager_.isCopyingInProgress();
}

std::vector<std::string> DrumController::getSuccessfulCopies()
{
    return file_manager_.getSuccessfulCopies();
}

std::vector<std::pair<std::string, std::string>> DrumController::getCopyErrors()
{
    return file_manager_.getCopyErrors();
}

int DrumController::getCopyQueueRemaining()
{
    return file_manager_.getCopyQueueRemaining();
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

            if (ma_sound_init_from_file(&engine_, samples_paths_[i].string().c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, sounds_[i]) == MA_SUCCESS)
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
            tracks_[i] = DrumTrackModel("", std::filesystem::path());
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
    auto bpmToMs = std::chrono::duration_cast<std::chrono::milliseconds>(secondsPerBeat / current_note_duration_mult_);

    if (is_playing_ && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStep_) > bpmToMs)
    {
        for (size_t i = 0; i < NUM_TRACKS; i++)
        {
            auto &track = tracks_[i];
            auto sequencer = track.getTrackSequencer();

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

void DrumController::setMasterVolume(float value)
{
    volume_ = value;
    ma_engine_set_gain_db(&engine_, value);
}

float DrumController::getMasterVolume()
{
    return ma_engine_get_gain_db(&engine_);
}

ma_sound *DrumController::getSound(int index)
{
    return sounds_.at(index);
}

void DrumController::setSoundVolume(int track_idx, float value)
{
    ma_sound *pSound = sounds_.at(track_idx);
    float linear_value = ma_volume_db_to_linear(value);
    ma_sound_set_volume(pSound, linear_value);
    track_volumes_.at(track_idx) = value;
}

float DrumController::getSoundVolume(int track_idx)
{
    ma_sound *pSound = sounds_.at(track_idx);
    float linear_volume = ma_sound_get_volume(pSound);
    return ma_volume_linear_to_db(linear_volume);
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
            samples_paths_.push_back(path);
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

void DrumController::addDrumPack(std::string name, std::array<std::filesystem::path, NUM_TRACKS> samples)
{
    int new_id = 0;
    if (!drum_packs_.empty())
    {
        int max_id = 0;
        for (const auto &pack : drum_packs_)
        {
            if (pack.id > max_id)
            {
                max_id = pack.id;
            }
        }
        new_id = max_id + 1;
    }

    DrumPack new_pack = drum_pack_manager_.createDrumPack(new_id, name, samples);
    drum_pack_manager_.saveDrumPack(new_pack);

    scanDrumPacks();

    int new_pack_index = getDrumPackIdx(name);
    if (new_pack_index >= 0)
    {
        setDrumPack(new_pack_index);
    }
}

std::filesystem::path DrumController::getSamplesRootDir()
{
    return samples_root_dir_;
}

void DrumController::deleteDrumPack(int index)
{
    if (index < 0 || index >= static_cast<int>(drum_packs_.size()))
    {
        return;
    }

    drum_pack_manager_.deleteDrumPack(drum_packs_.at(index));

    scanDrumPacks();

    if (drum_packs_.empty())
    {
        curr_drum_pack_index_ = -1;
    }
    else
    {
        if (index >= static_cast<int>(drum_packs_.size()))
        {
            index = static_cast<int>(drum_packs_.size()) - 1;
        }
        curr_drum_pack_index_ = index;
        setDrumPack(index);
    }
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
        }
    }
}

void DrumController::addPreset(Preset preset)
{
    std::string preset_file_path = (std::filesystem::current_path() / L"data" / "presets" / (StringUtils::toSnakeCase(preset.getPresetName()) + ".json")).string();
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
    std::string preset_file_path = (std::filesystem::current_path() / L"data" / "presets" / (StringUtils::toSnakeCase(preset_to_delete.getPresetName()) + ".json")).string();
    Preset::deletePresetFile(preset_file_path);

    // update presets_list_
    scanPresets();
}

// Change

int DrumController::getCurrentNoteDurationMult()
{
    return current_note_duration_mult_;
}

void DrumController::setNoteDuration(NoteDuration duration)
{
    switch (duration)
    {
    case NoteDuration::QUARTER:
        current_note_duration_mult_ = QUARTER_NOTE_MULT_;
        break;
    case NoteDuration::EIGHTH:
        current_note_duration_mult_ = EIGHT_NOTE_MULT_;
        break;
    case NoteDuration::SIXTEENTH:
        current_note_duration_mult_ = SIXTEENTH_NOTE_MULT_;
        break;
    default:
        current_note_duration_mult_ = SIXTEENTH_NOTE_MULT_;
        break;
    }
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

void DrumController::swapTracks(int index1, int index2)
{
    if (index1 < 0 || index1 >= NUM_TRACKS || index2 < 0 || index2 >= NUM_TRACKS)
    {
        return;
    }

    if (index1 == index2)
    {
        return;
    }

    std::swap(tracks_.at(index1), tracks_.at(index2));
    std::swap(sounds_.at(index1), sounds_.at(index2));
    std::swap(sound_initialized_.at(index1), sound_initialized_.at(index2));
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
