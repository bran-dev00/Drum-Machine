#include "drum_controller.hpp"
#include <string>
#include <iostream>


DrumController::DrumController(const std::string &samplePath)
    : samplePath_(samplePath)
{
    initSequencer();
    isPlaying_ = false;
    lastStep_ = std::chrono::steady_clock::now();
    beatCounter_ = 0;
    bpm_ = 120;

    ma_engine_init(NULL, &engine_);
}

DrumController::~DrumController() = default;

void DrumController::initSequencer()
{
    sequencerArr.fill(false);
    
    //TODO: Refactor
    tracks_.at(0).fill(false);
    tracks_.at(1).fill(false);
}

void DrumController::playSound(std::string &samplePath){
    ma_engine_play_sound(&engine_, samplePath.c_str(),NULL);
}

void DrumController::step()
{
    auto now = std::chrono::steady_clock::now();

    std::chrono::duration<double> secondsPerBeat(60.0 / bpm_);
    auto bpmToMs = std::chrono::duration_cast<std::chrono::milliseconds>(secondsPerBeat);

    if (isPlaying_ && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStep_) > bpmToMs)
    {
        // play sound if its marked in the sequencer array
        if (sequencerArr.at(beatCounter_) == true)
        {
            playSound(samplePath_);
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

void DrumController::setSequencerNoteTrue(Track_t &track, int index)
{
    if (index < 0 || index > MAX_STEPS - 1)
    {
        return;
    }
    track[index] = true;
    // sequencerArr[index] = true;
}

void DrumController::setSequencerNoteFalse(Track_t &track, int index)
{
    if (index < 0 || index > MAX_STEPS - 1)
    {
        return;
    }
    track[index] = false;
    // sequencerArr[index] = false;
}

void DrumController::resetSequencer(){
    sequencerArr.fill(false);
    beatCounter_ = 0;
    isPlaying_ = false;
}

int &DrumController::getBeatCounter()
{
    return this->beatCounter_;
}

bool DrumController::getIsPlaying()
{
    return this->isPlaying_;
}

std::array<Track_t, 2> &DrumController::getTracks(){
    return tracks_;
}

Track_t &DrumController::getTrackByIndex(int index){
        if (index < 0 || index >= static_cast<int>(tracks_.size())) {
            throw "index out of bounds";
        }

        return tracks_.at(index);
   
}

// debug string
std::string DrumController::getSequencerString()
{
    std::string output_string = "S ";

    for (int i = 0; i < MAX_STEPS; i++)
    {
        if (sequencerArr[i] == true)
        {
            output_string += "[X] ";
        }
        else
        {
            output_string += "[ ] ";
        }
    }

    output_string += "E";

    return output_string;
}

std::array<bool, MAX_STEPS> &DrumController::getSequencerArray()
{
    return sequencerArr;
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
