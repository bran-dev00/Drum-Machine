#pragma once

#include "drum_controller.hpp"
#include <filesystem>
#include "imgui.h"
#include <iostream>
#include <array>

class DrumView
{
private:
    DrumController &drum_controller_;
    ImGuiStyle &styles_ = ImGui::GetStyle();

public:
    DrumView(DrumController &controller);
    ~DrumView();

    void drawBeatCounterLabels(const std::array<float, MAX_STEPS> &positions);
    void drawTrack(int id, Track_t &track, std::array<float, MAX_STEPS> &checkbox_positions);
    void drawTracks(int num_tracks);
    void draw();
};
