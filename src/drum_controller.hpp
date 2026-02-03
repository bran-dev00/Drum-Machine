
#pragma once

#include <Windows.h>
#include<mmsystem.h>
#include <filesystem>
#include <chrono>
#include <string>

#define MAX_STEPS 16

using namespace std::chrono_literals;
class DrumController
{
private:
    bool sequencer[MAX_STEPS];
    int currStep;
    bool isPlaying;
public:
    DrumController();
    ~DrumController();

    void initSequencer();
    void setSequencerNoteTrue(int index);
    void setSequencerNoteFalse(int index);
    std::string getSequencer();

    void setIsPlayingFalse();
    void setIsPlayingTrue();

    //Step to next beat
    void step(std::chrono::time_point<std::chrono::steady_clock>, std::wstring& samplePath);

    void playSound(std::wstring& samplePath);
    void playSequencer(std::wstring& samplePath);
    void pauseSequencer();
};





