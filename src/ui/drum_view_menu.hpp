#pragma once
#include "drum_controller.hpp"
#include "presets.hpp"
#include "utils.hpp"
#include <filesystem>
#include "imgui.h"
#include <string>
#include <array>
#include <vector>
#include <set>

class DrumViewMenu
{
private:
    DrumController &drum_controller_;

    bool open_add_samples_modal_ = false;
    bool open_copy_progress_modal_ = false;
    bool open_copy_completion_modal_ = false;

    static char rename_input_buffer_[256];

    std::vector<path_pair_t> files_dropped_buf;
    std::set<path_pair_t> files_accepted_;

public:
    DrumViewMenu(DrumController &drum_controller) : drum_controller_(drum_controller) {};

    Preset savedCurrentPreset(std::string preset_name);
    bool isAddSamplesModalOpen() const { return open_add_samples_modal_; }

    void onFilesDropped(int count, const char **paths);

    void drawDrumPackSelectionMenu();
    void drawDrumPackCreationMenu();
    void drawFileMenu();
    void drawPresetsMenu();
    void drawDeleteSubMenu();
    void drawSavePresetPopup();

    void drawAddSamplesModal();
    void drawCopyConflictModal();
    void drawCopyProgressModal();
    void drawCopyCompletionModal();

    void drawMenuBar();
};
