#pragma once
#include "imgui.h"
#include "drum_controller.hpp"
#include "ui/drum_view_utils.hpp"

class DrumViewTracks
{

private:
    DrumController &drum_controller_;

public:
    DrumViewTracks(DrumController &drum_controller) : drum_controller_(drum_controller) {};

    void drawBeatIndicator(float width);
    void drawBeatCounterLabels(const std::array<float, MAX_STEPS> &positions);
    void drawResetAllButton();

    void drawTrack(int track_idx, Track_t &track, std::array<float, MAX_STEPS> &checkbox_positions, float checkbox_size);
    void drawTracks(float width);
};