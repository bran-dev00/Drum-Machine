#include "drum_controller.hpp"
#include <string>
#include <iostream>

DrumController::DrumController()
{
    isPlaying_ = false;
    lastStep_ = std::chrono::steady_clock::now();
    beatCounter_ = 0;
    bpm_ = 120;

    ma_engine_init(NULL, &engine_);
    loadInitialSamples();
    initSequencer();
}

DrumController::~DrumController() = default;

void DrumController::loadInitialSamples()
{
    // Default Samples
    std::string file_path_base = (std::filesystem::current_path()/L"assets").string();
    std::string sample_path;
    
    for (const auto &entry: std::filesystem::directory_iterator(file_path_base)){
        std::cout << "file: " << entry.path() << std::endl;
        std::string file_name = entry.path().string();
        samples_.push_back(file_name);
    }
} 

void DrumController::initSequencer()
{
    tracks_[0] = DrumTrackModel("track_1", samples_.at(0));
    tracks_[1] = DrumTrackModel("track_2", samples_.at(1));
    tracks_[2] = DrumTrackModel("track_3", samples_.at(2));
    tracks_[3] = DrumTrackModel("track_4", samples_.at(3));
}

void DrumController::playSound(std::string &samplePath)
{
    ma_engine_play_sound(&engine_, samplePath.c_str(), NULL);
}

void DrumController::step()
{
    auto now = std::chrono::steady_clock::now();

    std::chrono::duration<double> secondsPerBeat(60.0 / bpm_);
    auto bpmToMs = std::chrono::duration_cast<std::chrono::milliseconds>(secondsPerBeat);

    if (isPlaying_ && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStep_) > bpmToMs)
    {
        // play sound if its marked in the sequencer array
        if (tracks_[0].getTrackSequencer().at(beatCounter_) == true && !tracks_.empty())
        {
            auto sample = tracks_[0].getSample();
            // std::cout << "sample in track 1: " << sample <<std::endl;
            playSound(sample);
        }

        if (tracks_[1].getTrackSequencer().at(beatCounter_) == true && !tracks_.empty())
        {
            auto sample = tracks_[1].getSample();
            // std::cout << "sample in track 2: " << sample <<std::endl;
            playSound(sample);
        }
        if (tracks_[2].getTrackSequencer().at(beatCounter_) == true && !tracks_.empty())
        {
            auto sample = tracks_[2].getSample();
            // std::cout << "sample in track 2: " << sample <<std::endl;
            playSound(sample);
        }
        if (tracks_[3].getTrackSequencer().at(beatCounter_) == true && !tracks_.empty())
        {
            auto sample = tracks_[3].getSample();
            // std::cout << "sample in track 2: " << sample <<std::endl;
            playSound(sample);
        }


        lastStep_ = now;
        beatCounter_ = (beatCounter_ + 1) % MAX_STEPS;
    }
    else
    {
        return;
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

std::array<DrumTrackModel, 4> &DrumController::getTracks()
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
