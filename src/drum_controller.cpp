#include "drum_controller.hpp"
#include <string>
#include <iostream>

DrumController::DrumController()
{
    isPlaying_ = false;
    lastStep_ = std::chrono::steady_clock::now();
    beatCounter_ = 0;
    bpm_ = 90;

    ma_engine_init(NULL, &engine_);
    ma_engine_set_volume(&engine_, .5f);

    loadInitialSamples();
    initSoundArray();
    initSequencer();
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

void DrumController::loadInitialSamples()
{
    // Default Samples
    std::string file_path_base = (std::filesystem::current_path() / L"assets").string();
    std::string sample_path;

    for (const auto &entry : std::filesystem::directory_iterator(file_path_base))
    {
        std::string file_name = entry.path().string();
        samples_.push_back(file_name);
    }
}

// String Helper Function
static std::string extractSampleName(std::string file_path)
{

    std::string output_str;
    output_str = file_path.erase(0, file_path.find_last_of("\\") + 1);
    output_str = output_str.erase(output_str.find_first_of("."));

    return output_str;
}

void DrumController::initSoundArray()
{
    for (size_t i = 0; i < sounds_.size(); i++)
    {
        sounds_[i] = new ma_sound();
    }
}

void DrumController::initSequencer()
{
    sound_initialized_.fill(false);

    for (size_t i = 0; i < tracks_.size(); i++)
    {
        std::string track_name;
        track_name = extractSampleName(samples_[i]);
        tracks_[i] = DrumTrackModel(track_name, samples_[i]);

        if (ma_sound_init_from_file(&engine_, samples_[i].c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, sounds_[i]) == MA_SUCCESS)
        {
            sound_initialized_[i] = true;
        }
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

    if (isPlaying_ && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStep_) > bpmToMs)
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

void DrumController::setVolume(float value = .5f)
{
    if (value < 0)
    {
        volume_ = 0;
    }
    // TODO: Volume Limit
    volume_ = value;
    ma_engine_set_volume(&engine_, value);
}

float DrumController::getVolume()
{
    return ma_engine_get_volume(&engine_);
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
    isPlaying_ = false;
    beatCounter_ = 0;
}

int &DrumController::getBeatCounter()
{
    return this->beatCounter_;
}

bool DrumController::getIsPlaying()
{
    return this->isPlaying_;
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

void DrumController::playSequencer()
{
    isPlaying_ = true;
}

void DrumController::pauseSequencer()
{
    isPlaying_ = false;
}

void DrumController::toggleSequencer()
{
    isPlaying_ = !isPlaying_;
}
