#include "drum_controller.hpp"
#include <string>
#include <iostream>

// Demo Preset

DrumController::DrumController()
{
    is_playing_ = false;
    lastStep_ = std::chrono::steady_clock::now();
    beatCounter_ = 0;
    bpm_ = 90;
    base_assets_dir_ = (std::filesystem::current_path() / L"assets").string();
    curr_drum_pack_ = (std::filesystem::current_path() / L"assets" / L"Kit-1").string();
    main_session_file_path_ = (std::filesystem::current_path() / L"data" / L"main_session.txt").string();

    ma_engine_init(NULL, &engine_);
    ma_engine_set_volume(&engine_, .5f);

    // first initialization
    sound_initialized_.fill(false);
    scanDrumPacks();
    loadInitialSamples();
    initSoundArray();
    initSequencer();
    initTrackVolumesArr();

    initDemoPreset();
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

void DrumController::initDemoPreset()
{
    std::array<Track_t, NUM_TRACKS> preset_tracks;
    std::array<std::string, NUM_TRACKS> demo_preset_tracks = {
        "1000 0000 0000 0000",
        "0010 0001 1000 0110",
        "1111 1111 1111 1111",
        "0000 0000 0000 0000",
        "1000 1000 1000 1000",
        "0111 1110 1111 1100",
        "0000 0000 0000 0000",
        "0010 0110 0100 1001"};

    for (size_t i = 0; i < NUM_TRACKS; i++)
    {
        preset_tracks.at(i) = Preset::parseStringPattern(demo_preset_tracks.at(i));
    }

    Preset demo_preset = {"Demo Preset", 0, preset_tracks, 90, track_volumes_};
    presets_list_.push_back(demo_preset);
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
    std::string sample_path = (std::filesystem::current_path() / L"assets" / L"Kit-1").string();
    loadSamples(sample_path);
}

std::string DrumController::extractSampleName(std::string file_path)
{
    std::string output_str;
    output_str = file_path.erase(0, file_path.find_last_of("\\") + 1);
    output_str = output_str.erase(output_str.find_first_of("."));

    return output_str;
}

std::string DrumController::extractDirName(std::string file_path)
{
    std::string output_str;
    output_str = file_path.erase(0, file_path.find_last_of("\\") + 1);

    return output_str;
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
            track_name = extractSampleName(samples_paths_[i]);
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

void DrumController::scanDrumPacks()
{
    for (const auto &entry : std::filesystem::directory_iterator(base_assets_dir_))
    {
        if (entry.is_directory())
        {
            drum_packs_.push_back(entry.path().string());
        }
    }
}

int DrumController::getDrumPackIdx(std::string drum_pack_path)
{
    for (size_t i = 0; i < drum_packs_.size(); i++)
    {
        if (drum_packs_.at(i) == drum_pack_path)
        {
            return i;
        }
    }

    return -1;
}

void DrumController::setDrumPack(int index)
{
    if (index < 0 || index > drum_packs_.size())
    {
        std::cout << "drum_packs_ index out of bounds!\n";
        return;
    }

    if (curr_drum_pack_ != drum_packs_.at(index))
    {
        is_playing_ = false;
        beatCounter_ = 0;
        curr_drum_pack_ = drum_packs_.at(index);
        loadSamples(curr_drum_pack_);
        // update sounds here
        initSequencer();
    }
    return;
}

std::string DrumController::getCurrDrumPack()
{
    return curr_drum_pack_;
}

std::vector<std::string> DrumController::getDrumPacks()
{
    return drum_packs_;
}

void DrumController::loadPreset(int index)
{
    if (presets_list_.empty())
    {
        return;
    }

    Preset preset = presets_list_.at(index);
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
            Preset preset = Preset::parsePresetFromFile(file_path);
            presets_list_.push_back(preset);

            // DEBUG
            /*   for (size_t i = 0; i < presets_list_.size(); i++)
              {
                  std::cout << "list size: " << presets_list_.size() << "\n";
                  std::cout << "Preset " << i << ": " << presets_list_.at(i).getPresetName() << "\n";
              } */
        }
    }
}

void DrumController::addPreset(Preset preset)
{
    std::string preset_file_path = (std::filesystem::current_path() / L"data" / "presets" / (preset.getPresetName() + ".txt")).string();
    Preset::savePresetToFile(preset, preset_file_path);

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
    std::string preset_file_path = (std::filesystem::current_path() / L"data" / "presets" / (preset_to_delete.getPresetName() + ".txt")).string();
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

void DrumController::saveSession()
{
    std::string data_dir = (std::filesystem::current_path() / L"data").string();

    std::ofstream session_file(data_dir + "/main_session.txt");
    if (session_file.is_open())
    {
        session_file << "BPM: " << bpm_ << "\n";

        for (size_t i = 0; i < drum_packs_.size(); i++)
        {
            auto drum_pack_path = drum_packs_.at(i);
            session_file << "Drum Pack " << i << ": " << drum_pack_path << "\n";
        }

        session_file.close();
    }
    else
    {
        std::cout << "Failed to open session file for saving!\n";
        return;
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
