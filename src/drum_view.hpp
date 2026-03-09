#pragma once
#include "drum_controller.hpp"
#include "presets.hpp"
#include <filesystem>
#include "imgui.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <array>
#include <vector>
#include <iostream>

#define SCR_SM 800
#define SCR_MD 1024
#define SCR_LG 1440
#define SCR_XL 1920
#define SCR_XXL 2560

class DrumView
{
private:
    DrumController &drum_controller_;
    ImGuiStyle &styles_ = ImGui::GetStyle();
    ImGuiIO &io = ImGui::GetIO();
    const ImVec2 base_resolution_ = ImVec2(1920.0f, 1080.0f);

    float getScaleFactor() const
    {
        return io.DisplaySize.x / base_resolution_.x;
    }

    float getPartitionSize() const
    {
        return io.DisplaySize.x / 8.0f;
    }

public:
    DrumView(DrumController &controller);
    ~DrumView();

    // Temp
    void drawDebug();

    void drawHoverCursor();

    void drawBeatCounterLabels(const std::array<float, MAX_STEPS> &positions);
    void drawTrack(int id, Track_t &track, std::array<float, MAX_STEPS> &checkbox_positions, float checkbox_size);
    void drawTracks(float width);

    void drawBeatIndicator(float width);
    void drawCustomVolumeSlider(const std::string label, int track_idx, float &value, float v_min, float v_max);

    void drawDrumPackSelectionMenu();
    void drawFileMenu();
    void drawSavePresetPopup();
    void drawDeleteSubMenu();
    void drawPresetsMenu();

    void drawMenuBar();

    void drawBpmControls(int &bpm);
    void drawMasterVolume(float &volume);
    void drawTogglePlayButton();

    void drawControls(float start_x);
    void drawResetAllButton();
    void drawMainContainer(float start_x, float width);

    void draw();
};
