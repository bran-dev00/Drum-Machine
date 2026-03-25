#pragma once
#include "drum_controller.hpp"
#include "utils.hpp"
#include "presets.hpp"
#include <filesystem>
#include "imgui.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <array>
#include <vector>
#include <set>
#include <iostream>
#include "ui/drum_view_controls.hpp"
#include "ui/drum_view_tracks.hpp"
#include "ui/drum_view_menu.hpp"

#define SCR_SM 800
#define SCR_MD 1024
#define SCR_LG 1440
#define SCR_XL 1920
#define SCR_XXL 2560

class DrumView
{
private:
    DrumController &drum_controller_;
    DrumViewControls drum_view_controls_;
    DrumViewTracks drum_view_tracks_;
    DrumViewMenu drum_view_menu_;

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

    void onFilesDropped(int count, const char **paths);
    Preset savedCurrentPreset(std::string preset_name);

    void drawHoverCursor();
    void drawControls(float start_x);
    void drawMainContainer(float start_x, float width);
    void draw();
};
