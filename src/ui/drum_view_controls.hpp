#pragma once
#include "drum_controller.hpp"
#include "imgui.h"
#include "ui/drum_view_utils.hpp"

class DrumViewControls
{
private:
    DrumController &drum_controller_;

public:
    DrumViewControls(DrumController &drum_controller) : drum_controller_(drum_controller) {};

    void drawBpmControls(int &bpm);
    void drawMasterVolume(float &volume);
    void drawTogglePlayButton();
    void drawControls(float start_x);
};