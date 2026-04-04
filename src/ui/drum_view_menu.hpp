#pragma once
#include "drum_controller.hpp"
#include "ui/drum_view_utils.hpp"
#include "presets.hpp"
#include "utils.hpp"
#include <filesystem>
#include "imgui.h"
#include <string>
#include <array>
#include <vector>
#include <set>
#include <map>

struct SampleSelection
{
    std::string folder_name;
    std::map<std::string, bool> sample_selections;
};

class DrumViewMenu
{
private:
    DrumController &drum_controller_;

    bool open_add_samples_modal_ = false;
    bool open_copy_progress_modal_ = false;
    bool open_copy_completion_modal_ = false;
    bool open_create_drum_pack_modal_ = false;
    bool open_rearrange_tracks_modal_ = false;
    bool open_save_preset_modal_ = false;

    static char rename_input_buffer_[256];

    std::vector<path_pair_t> files_dropped_buf;
    std::set<path_pair_t> files_accepted_;

    FileUtils::SampleDirectoryStructure sample_structure_;

    std::map<std::string, bool> root_sample_selections_;

    std::vector<SampleSelection> folder_selections_;
    std::string new_drum_pack_name_;
    int delete_drum_pack_selected_index_ = 0;

public:
    DrumViewMenu(DrumController &drum_controller) : drum_controller_(drum_controller) {};

    Preset savedCurrentPreset(std::string preset_name);
    bool isAddSamplesModalOpen() const { return open_add_samples_modal_; }

    void onFilesDropped(int count, const char **paths);

    void drawDrumPackSelectionMenu();
    void drawDrumPackCreationMenu();
    void drawDeleteDrumPackSubMenu();
    void drawFileMenu();
    void drawPresetsMenu();
    void drawDeleteSubMenu();
    void drawSavePresetModal();

    void drawRearrangeTracksModal();
    void drawAddSamplesModal();
    void drawCopyConflictModal();
    void drawCopyProgressModal();
    void drawCopyCompletionModal();
    void drawCreateDrumPackModal();

    void drawMenuBar();
};
