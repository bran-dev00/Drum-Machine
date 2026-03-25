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

#define SCR_SM 800
#define SCR_MD 1024
#define SCR_LG 1440
#define SCR_XL 1920
#define SCR_XXL 2560

class DrumView
{
private:
    DrumController &drum_controller_;
    DrumViewControls controls_;
    ImGuiStyle &styles_ = ImGui::GetStyle();
    ImGuiIO &io = ImGui::GetIO();
    const ImVec2 base_resolution_ = ImVec2(1920.0f, 1080.0f);

    bool open_add_samples_modal_ = false;
    bool open_copy_progress_modal_ = false;
    bool open_copy_completion_modal_ = false;

    static char rename_input_buffer_[256];

    // Buffer
    std::vector<path_pair_t> files_dropped_buf;

    // Used for rendering, no duplicates
    std::set<path_pair_t> files_accepted_;

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

    void drawDrumPackSelectionMenu();
    void drawDrumPackCreationMenu();

    void onFilesDropped(int count, const char **paths);
    void drawAddSamplesModal();

    void drawCopyConflictModal();
    void drawCopyProgressModal();
    void drawCopyCompletionModal();

    void drawFileMenu();
    Preset savedCurrentPreset(std::string preset_name);
    void drawSavePresetPopup();
    void drawDeleteSubMenu();
    void drawPresetsMenu();

    void drawMenuBar();
    void drawControls(float start_x);
    void drawResetAllButton();
    void drawMainContainer(float start_x, float width);

    void draw();
};
