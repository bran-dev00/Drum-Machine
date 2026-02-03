#include "drum_controller.hpp"
#include <string>
#include <iostream>

DrumController::DrumController()
{
    initSequencer();
    isPlaying = false;
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

void DrumController::step(std::chrono::time_point<std::chrono::steady_clock> lastStep, std::wstring& samplePath){
    auto now = std::chrono::steady_clock::now();
    
    //TODO: Match time with set bpm
    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStep) > 1000ms){
        playSound(samplePath);
    }else{
        return;
    }

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

void DrumController::playSequencer(std::wstring& samplePath) 
{
    setIsPlayingTrue();
    
    std::chrono::time_point<std::chrono::steady_clock> last_sound_timepoint = std::chrono::steady_clock::now(); 
    int test_stop = 0;

    while(isPlaying)
    {
        auto now = std::chrono::steady_clock::now();
        
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_sound_timepoint) > 1000ms ){
            currStep = (currStep + 1) % MAX_STEPS;

            if(test_stop >= 10){
                pauseSequencer();
            }

            if (sequencer[currStep]){
                PlaySoundW(samplePath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
                std::cout << "PLAY SOUND: "<< isPlaying << std::endl;
            }
            last_sound_timepoint = now;
            test_stop++;
        }
        // std::cout << "DEBUG: OUT HERE: "<<  isPlaying << std::endl;
    }
}

void DrumController::pauseSequencer()
{
    DrumController::setIsPlayingFalse();
    PlaySound(NULL, 0, 0);
}
