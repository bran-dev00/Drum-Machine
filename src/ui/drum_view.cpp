#include "drum_view.hpp"
#include <imgui_internal.h>
#include <cstring>

char DrumView::rename_input_buffer_[256];

DrumView::DrumView(DrumController &controller) : drum_controller_(controller), drum_view_controls_(controller), drum_view_tracks_(controller)
{
    // Global Styles
    styles_.FrameRounding = 3.0f;
    ImGuiIO &io = ImGui::GetIO();

    // Scale
    float scale = getScaleFactor();
    float font_size = std::clamp(16.0f * scale, 16.0f, 32.0f);

    std::string lexend_font = (std::filesystem::current_path() / L"fonts" / L"Lexend-Medium.ttf").string();
    io.Fonts->AddFontFromFileTTF(lexend_font.c_str(), font_size);

    memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));
}

DrumView::~DrumView() = default;

void DrumView::drawHoverCursor()
{

    ImU32 color_fill = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 color_border = ImGui::GetColorU32(ImVec4(255.0f, 255.0f, 255.0f, 1.0f));

    // Global Hover Cursor
    if (ImGui::IsAnyItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        ImGui::RenderMouseCursor(ImGui::GetMousePos(), 1.0f, ImGuiMouseCursor_Hand, color_fill, color_border, ImGui::GetColorU32(ImVec4(0, 0, 0, 0.5f)));
    }
}

void DrumView::drawControls(float start_x)
{
    drum_view_controls_.drawControls(start_x);
}

void DrumView::drawTracks(float width)
{
    drum_view_tracks_.drawTracks(width);
}

void DrumView::drawDrumPackSelectionMenu()
{
    std::string curr_drum_pack = PathUtils::extractDirName(drum_controller_.getCurrDrumPack());
    std::vector<std::string> drum_packs = drum_controller_.getDrumPacks();

    // extract drum pack names
    for (size_t i = 0; i < drum_packs.size(); i++)
    {
        std::string d_name = PathUtils::extractDirName(drum_packs.at(i));
        drum_packs.at(i) = d_name;
    }

    static int selected = 0;
    for (size_t i = 0; i < drum_packs.size(); i++)
    {
        std::string name = drum_packs.at(i);
        if (ImGui::Selectable(name.c_str(), selected == i))
        {
            selected = i;
            drum_controller_.setDrumPack(i);
        }
    }
}

void DrumView::drawDrumPackCreationMenu()
{
    if (ImGui::Selectable("Create New Drum Pack"))
    {
        std::cout << "pressed\n";
    }
}

// callback function
void DrumView::onFilesDropped(int count, const char **paths)
{

    // Only allow if the modal is open
    if (open_add_samples_modal_ == false)
    {
        return;
    }

    files_dropped_buf.clear();

    for (int i = 0; i < count; i++)
    {
        std::filesystem::path p = std::filesystem::path(paths[i]);

        if (std::filesystem::is_directory(p))
        {
            auto files_with_paths = FileValidation::getFilesWithRelativePaths(p);
            for (auto &file : files_with_paths)
            {
                files_dropped_buf.push_back(std::move(file));
            }
        }
        else
        {
            auto validated_files = FileValidation::validateFile(p);
            for (auto &file : validated_files)
            {
                files_dropped_buf.push_back({file, file.filename()});
            }
        }
    }
}

void DrumView::drawCopyConflictModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("CopyConflict", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        std::string conflict_name = drum_controller_.getCurrentConflictFilename();
        ImGui::Text("File \"%s\" already exists.", conflict_name.c_str());
        ImGui::Spacing();

        ImGui::Text("Rename to:");
        ImGui::SameLine();

        std::string current_name = drum_controller_.getCurrentConflictFilename();
        std::string stem;
        if (!current_name.empty())
        {
            std::filesystem::path p(current_name);
            stem = p.stem().string();
        }

        // empty string
        if (rename_input_buffer_[0] == '\0')
        {
            strncpy(rename_input_buffer_, stem.c_str(), sizeof(rename_input_buffer_) - 1);
            rename_input_buffer_[sizeof(rename_input_buffer_) - 1] = '\0';
        }

        ImGui::InputText("##RenameInput", rename_input_buffer_, sizeof(rename_input_buffer_));

        ImGui::Spacing();

        if (ImGui::Button("Skip"))
        {
            drum_controller_.skipCurrentFile();
            // clear buffer
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        bool can_rename = strlen(rename_input_buffer_) > 0;

        // don't allow empty sample name
        ImGui::BeginDisabled(!can_rename);
        if (ImGui::Button("Rename"))
        {
            drum_controller_.renameAndCopyCurrentFile(std::string(rename_input_buffer_));
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Replace"))
        {
            drum_controller_.replaceCurrentFile();
            // clear buffer
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Skip All"))
        {
            drum_controller_.finishCopy();
            // clear buffer
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));

            open_copy_progress_modal_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumView::drawCopyProgressModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float y_offset = 100.0f;

    ImGui::SetNextWindowSize(ImVec2(avail.x / 2, avail.y / 4));
    ImGui::SetNextWindowPos(ImVec2(center.x, center.y + y_offset), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("CopyProgress", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        bool has_conflict = drum_controller_.hasCopyConflict();
        std::string copying_file = drum_controller_.getCurrentCopyingFilename();

        int remaining = drum_controller_.getCopyQueueRemaining();
        ImGui::Text("Copying: %s", copying_file.c_str());
        ImGui::Spacing();

        ImGui::Text("%d files remaining", remaining);
        ImGui::Spacing();

        if (ImGui::Button("Cancel"))
        {
            drum_controller_.finishCopy();
            open_copy_progress_modal_ = false;
            ImGui::CloseCurrentPopup();
        }

        if (has_conflict)
        {
            ImGui::OpenPopup("CopyConflict");
            drawCopyConflictModal();
        }

        ImGui::EndPopup();
    }
}

void DrumView::drawCopyCompletionModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("CopyCompletion", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        auto successful = drum_controller_.getSuccessfulCopies();
        auto errors = drum_controller_.getCopyErrors();

        ImGui::Text("Copy Complete!");
        ImGui::Spacing();

        ImGui::Text("Successfully copied: %zu file(s)", successful.size());

        if (!errors.empty())
        {
            ImGui::Text("Errors: %zu", errors.size());
            for (const auto &[file, msg] : errors)
            {
                ImGui::BulletText("%s: %s", file.c_str(), msg.c_str());
            }
        }

        ImGui::Spacing();

        if (ImGui::Button("Done"))
        {
            open_copy_completion_modal_ = false;
            open_copy_progress_modal_ = false;
            open_add_samples_modal_ = false;

            drum_controller_.finishCopy();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumView::drawAddSamplesModal()
{

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("AddSamples", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Drop Your Samples Here");

        ImGui::SameLine();
        ImGui::TextDisabled("(?)");

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Double click the file to remove");
        }

        ImVec2 list_box_size = ImVec2(0, 0);
        static float max_text_width = 0.0f;

        for (auto &file : files_dropped_buf)
        {
            if (std::filesystem::exists(file.first))
            {
                float w = ImGui::CalcTextSize(file.first.string().c_str()).x;

                max_text_width = (std::max)(max_text_width, w);
                files_accepted_.insert(std::move(file));
            }
        }

        files_dropped_buf.clear();

        if (files_accepted_.size() > 0)
        {
            float padding_x = 15.0f;
            list_box_size.x = max_text_width + padding_x;
        }

        // TODO:Extract Later?
        if (ImGui::BeginListBox("##Dropped Samples", list_box_size))
        {
            std::set<path_pair_t>::iterator to_erase = files_accepted_.end();

            int idx = 0;
            for (auto it = files_accepted_.begin(); it != files_accepted_.end(); ++it, ++idx)
            {
                const auto &file = *it;

                if (ImGui::Selectable(file.first.string().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        to_erase = it;
                    }
                }
            }

            if (to_erase != files_accepted_.end())
            {
                files_accepted_.erase(to_erase);
            }

            ImGui::EndListBox();
        }

        ImGui::BeginDisabled(files_accepted_.empty());
        if (ImGui::Button("Submit"))
        {
            drum_controller_.startCopyQueue(files_accepted_);
            files_accepted_.clear();
            open_add_samples_modal_ = false;
            open_copy_progress_modal_ = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            files_accepted_.clear();
            files_dropped_buf.clear();
            ImGui::CloseCurrentPopup();
            open_add_samples_modal_ = false;
        }

        ImGui::EndPopup();
    }
}

void DrumView::drawFileMenu()
{
    Preset main_session = savedCurrentPreset("Main Session");
    if (ImGui::MenuItem("Save Session", NULL, false, true))
    {
        drum_controller_.saveSession(main_session);
    }

    if (ImGui::MenuItem("Add Samples", NULL, false, true))
    {
        open_add_samples_modal_ = true;
    }
}

void DrumView::drawDeleteSubMenu()
{

    auto presets_list = drum_controller_.getPresetsList();

    static int selected = 0;
    for (size_t i = 0; i < presets_list.size(); i++)
    {
        std::string preset_name = presets_list.at(i).getPresetName();
        if (ImGui::Selectable(preset_name.c_str(), selected == i, ImGuiSelectableFlags_DontClosePopups))
        {
            selected = i;
        }
    }

    if (ImGui::Button("Delete Selected Preset"))
    {
        drum_controller_.deletePreset(selected);
        ImGui::CloseCurrentPopup();
    }
}

void DrumView::drawPresetsMenu()
{

    auto presets_list = drum_controller_.getPresetsList();

    static int selected = 0;
    for (size_t i = 0; i < presets_list.size(); i++)
    {
        std::string preset_name = presets_list.at(i).getPresetName();
        if (ImGui::Selectable(preset_name.c_str(), selected == i))
        {
            selected = i;
            drum_controller_.loadPreset(presets_list.at(i));
        }
    }
}

// Returns whatever the user currently has on screen as a preset
Preset DrumView::savedCurrentPreset(std::string preset_name)
{
    int bpm = drum_controller_.getBpm();
    int drum_pack_idx = drum_controller_.getDrumPackIdx(drum_controller_.getCurrDrumPack());
    std::array<float, NUM_TRACKS> track_volumes = drum_controller_.getTrackVolumes();

    auto tracks = drum_controller_.getTracks();
    std::array<Track_t, NUM_TRACKS> track_patterns;
    for (int i = 0; i < NUM_TRACKS; i++)
    {
        track_patterns.at(i) = tracks.at(i).getTrackSequencer();
    }

    Preset new_preset(preset_name, drum_pack_idx, track_patterns, bpm, track_volumes);
    return new_preset;
}

void DrumView::drawSavePresetPopup()
{
    if (ImGui::BeginPopup("SavePresetPopup"))
    {
        static char preset_name[64] = "New Preset";
        ImGui::InputText("Preset Name", preset_name, sizeof(preset_name));

        Preset new_preset = savedCurrentPreset(preset_name);

        if (ImGui::Button("Save Preset"))
        {
            drum_controller_.addPreset(new_preset);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumView::drawMenuBar()
{
    bool open_save_popup = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("File", NULL, false, false);
            drawFileMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Drum Packs"))
        {
            ImGui::MenuItem("Drum Packs", NULL, false, false);
            drawDrumPackSelectionMenu();

            ImGui::MenuItem("Create New Kit", NULL, false, false);
            drawDrumPackCreationMenu();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Presets"))
        {
            ImGui::MenuItem("Presets", NULL, false, false);
            drawPresetsMenu();
            ImGui::NewLine();
            ImGui::Separator();

            if (ImGui::MenuItem("Save Current as Preset", NULL, false, true))
            {
                open_save_popup = true;
            }

            if (ImGui::BeginMenu("Remove Presets"))
            {
                ImGui::MenuItem("Remove Presets", NULL, false, false);
                drawDeleteSubMenu();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (open_save_popup)
    {
        ImGui::OpenPopup("SavePresetPopup");
    }

    if (open_add_samples_modal_)
    {
        ImGui::OpenPopup("AddSamples");
    }

    if (open_copy_progress_modal_)
    {
        ImGui::OpenPopup("CopyProgress");
    }

    if (open_copy_completion_modal_)
    {
        ImGui::OpenPopup("CopyCompletion");
    }

    drawSavePresetPopup();
    drawAddSamplesModal();

    drawCopyProgressModal();
    drawCopyConflictModal();
    drawCopyCompletionModal();

    if (drum_controller_.isCopyingInProgress())
    {
        drum_controller_.processNextCopy();
    }

    if (drum_controller_.getCopyQueueRemaining() == 0 && !drum_controller_.hasCopyConflict())
    {
        if (open_copy_progress_modal_)
        {
            drum_controller_.finishCopy();
            open_copy_progress_modal_ = false;
            open_copy_completion_modal_ = true;
        }
    }
}

void DrumView::drawMainContainer(float start_x, float width)
{
    float scale = getScaleFactor();
    float partition = getPartitionSize();

    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 3.0f);
    ImU32 test_color = ImColor(0, 255, 255, 255);

    ImGui::SetCursorPosX(start_x);
    ImGui::BeginChild("##MainContainer", ImVec2(width, 0), false, ImGuiChildFlags_FrameStyle);

    drawControls(0);
    drawTracks(width);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void DrumView::draw()
{
    {
        ImVec2 display = ImGui::GetIO().DisplaySize;

        // fullscreen
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(display, ImGuiCond_Always);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::Begin("Drum Machine", NULL, window_flags);
        drawMenuBar();

        float partition_size = getPartitionSize();
        float scale = getScaleFactor();

        // Breakpoints
        float start_x = partition_size;
        float end_x = partition_size;

        if (display.x > 0 && display.x <= SCR_SM)
        {
            start_x = partition_size;
            end_x = partition_size * 7 - (partition_size / 2);
        }
        else if (display.x > SCR_SM && display.x <= SCR_MD)
        {
            start_x = partition_size;
            end_x = partition_size * 6;
        }
        else
        {
            start_x = partition_size * 2;
            end_x = partition_size * 4;
        }

        float y_offset = 25.0f;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y_offset);
        drawMainContainer(start_x, end_x);

        drawHoverCursor();
        ImGui::NewLine();

        ImGui::End();
    }
}
