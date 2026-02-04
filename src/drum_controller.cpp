#include "drum_controller.hpp"
#include <string>
#include <iostream>

DrumController::DrumController(const std::wstring& samplePath)
: samplePath(samplePath)
{
    
    initSequencer();
    isPlaying = false;
    lastStep = std::chrono::steady_clock::now();
    bpm_ = 120;

}

DrumController::~DrumController() = default;

void DrumController::initSequencer()
{
    for (int i = 0; i < MAX_STEPS; ++i)
        sequencer[i] = false;
}

void DrumController::setIsPlayingFalse(){
    isPlaying = false;
    
} 

void DrumController::setIsPlayingTrue(){
    isPlaying = true;
} 

void DrumController::playSound(std::wstring& samplePath)
{
    PlaySoundW(samplePath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
}

void DrumController::step(){
    auto now = std::chrono::steady_clock::now();
    
    //TODO: Match time with set bpm
    std::chrono::duration<double> secondsPerBeat(60.0/bpm_);
    auto bpmToMs = std::chrono::duration_cast<std::chrono::milliseconds>(secondsPerBeat);
    if(isPlaying && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStep) > bpmToMs){
        playSound(samplePath);
        lastStep = now;
    }else{
        return;
    }

}

void DrumController::setBpm(int bpm){
    bpm_ = bpm;
}

void DrumController::setSequencerNoteTrue(int index)
{
    if (index < 0 || index > MAX_STEPS-1){
        return;
    }
    sequencer[index] = true;
}

void DrumController::setSequencerNoteFalse(int index)
{
    if (index < 0 || index > MAX_STEPS-1){
        return;
    }
    sequencer[index] = false;
}

std::string DrumController::getSequencer()
{
    std::string output_string = "";
    
    for(int i =0; i< MAX_STEPS; i++){
        if(sequencer[i] == true){
            output_string += "X";
        }else{
            output_string += ".";
        }
    }
    
    return output_string;

}

void DrumController::playSequencer() 
{
    setIsPlayingTrue();
}

void DrumController::pauseSequencer()
{
    setIsPlayingFalse();
    PlaySound(NULL, 0, 0);
}
