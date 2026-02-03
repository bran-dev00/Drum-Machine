
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
    int bpm_;
    std::chrono::time_point<std::chrono::steady_clock> lastStep;
    std::wstring samplePath;
public:
    DrumController(const std::wstring& samplePath);
    ~DrumController();

    void initSequencer();
    void setSequencerNoteTrue(int index);
    void setSequencerNoteFalse(int index);
    std::string getSequencer();

    void setIsPlayingFalse();
    void setIsPlayingTrue();

    //Step to next beat
    void step();
    void setBpm(int bpm);

    void playSound(std::wstring& samplePath);
    void playSequencer();
    void pauseSequencer();
};





