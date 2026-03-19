#include "drum_view.hpp"
#include <imgui_internal.h>
#include <cstring>

char DrumView::rename_input_buffer_[256];

DrumView::DrumView(DrumController &controller) : drum_controller_(controller)
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

void DrumView::drawBeatCounterLabels(const std::array<float, MAX_STEPS> &positions)
{
    float y = ImGui::GetCursorScreenPos().y;

    for (size_t i = 0; i < positions.size(); ++i)
    {
        ImGui::AlignTextToFramePadding();
        std::string label = std::to_string(static_cast<int>(i) + 1);
        ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
        // positions contains the center X of each checkbox
        float x = positions[i] - (text_size.x / 2.0f);
        ImGui::SetCursorScreenPos(ImVec2(x, y));
        ImGui::TextUnformatted(label.c_str());
    }
}

void DrumView::drawCustomVolumeSlider(std::string label, int track_idx, float &value, float v_min, float v_max)
{

    ImVec2 p = ImGui::GetCursorScreenPos();
    float width = ImGui::CalcItemWidth() / 1.5f;
    float height = ImGui::GetFrameHeight() / 2.5f;

    ImU32 bg_color = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 handle_color = ImGui::GetColorU32(ImVec4(255.0f, 255.0f, 255.0f, 1.0f));

    ImGui::InvisibleButton(label.c_str(), ImVec2(width, height));

    bool is_active = ImGui::IsItemActive();
    bool clicked = ImGui::IsItemClicked();

    if (is_active)
    {
        float mouse_pos = ImGui::GetMousePos().x - p.x;
        value = v_min + (mouse_pos / width) * (v_max - v_min);
        value = ImClamp(value, v_min, v_max);

        if (track_idx == -1)
        {
            drum_controller_.setMasterVolume(value);
        }
        else
        {
            drum_controller_.setSoundVolume(track_idx, value);
        }
    }

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bg_color, height * 0.5f);
    float fill_width = ((value - v_min) / (v_max - v_min)) * width;
    draw_list->AddRectFilled(p, ImVec2(p.x + fill_width, p.y + height), handle_color, height * 0.5f);

    float handle_x = p.x + fill_width;
    draw_list->AddCircleFilled(ImVec2(handle_x, p.y + height * 0.5f), height * 0.75f, handle_color);
}

void DrumView::drawTrack(int track_idx, Track_t &track, std::array<float, MAX_STEPS> &checkbox_positions, float checkbox_size)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(checkbox_size, checkbox_size));

    for (int j = 0; j < MAX_STEPS; ++j)
    {
        std::string id = std::string("##track_") + std::to_string(track_idx) + "_beat_" + std::to_string(j);
        if (ImGui::Checkbox(id.c_str(), &track[j]))
        {
            if (j < MAX_STEPS)
            {
                if (track[j])
                {
                    drum_controller_.setSequencerNoteTrue(track, j);
                }
                else
                {
                    drum_controller_.setSequencerNoteFalse(track, j);
                }
            }
        }

        ImVec2 item_min = ImGui::GetItemRectMin();
        ImVec2 item_max = ImGui::GetItemRectMax();
        float center_x = (item_min.x + item_max.x) * 0.5f;
        checkbox_positions[j] = center_x;

        ImGui::SameLine();
    }
    ImGui::PopStyleVar();
}

void DrumView::drawTracks(float width)
{
    auto &tracks = drum_controller_.getTracks();
    std::array<float, NUM_TRACKS> track_volumes = drum_controller_.getTrackVolumes();

    // Store the checkbox positions to calculate label position
    std::array<float, MAX_STEPS> checkbox_positions{};

    float scale = getScaleFactor();
    float partition_size = getPartitionSize();
    float item_spacing = ImGui::GetStyle().ItemSpacing.x;
    float font_size = ImGui::GetFontSize();

    float checkbox_budget = width * (4.0f / 5.0f);

    float checkbox_size = (std::max)(1.0f, (checkbox_budget / MAX_STEPS - font_size - item_spacing) * 0.5f);

    drawBeatIndicator(width);

    ImGui::BeginChild("Tracks", ImVec2(width, 0), false, ImGuiWindowFlags_NoScrollbar);
    for (int i = 0; i < NUM_TRACKS; ++i)
    {
        ma_sound *sound = drum_controller_.getSound(i);

        std::string track_name = tracks[i].getName();
        ImGui::SeparatorText(track_name.c_str());

        Track_t &track = tracks.at(i).getTrackSequencer();

        ImGui::PushID(i);

        // std::cout << "checkbox_size: " << checkbox_size << "\n";
        drawTrack(i, track, checkbox_positions, checkbox_size);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + checkbox_size - 2.0f);
        if (ImGui::Button("Reset"))
        {
            drum_controller_.resetSequencer(track);
        }
        ImGui::SameLine();

        float button_height = ImGui::GetFrameHeight();
        float slider_height = button_height / 2;
        float vertical_offset = (button_height - slider_height) * 0.5f;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + vertical_offset + checkbox_size);
        ImGui::PushItemWidth(150.0f * scale);
        drawCustomVolumeSlider("Volume", i, track_volumes.at(i), 0, 1);
        ImGui::PopItemWidth();

        ImGui::PopID();

        ImGui::NewLine();
    }

    drawBeatCounterLabels(checkbox_positions);
    ImGui::SameLine(0.0f, 10.0f);
    drawResetAllButton();
    ImGui::EndChild();
}

void DrumView::drawBeatIndicator(float width)
{
    float checkbox_width = ImGui::GetFrameHeightWithSpacing() * 1.5f;
    ImGui::PushItemWidth(width);
    ImGui::SliderInt("##Beat", &drum_controller_.getBeatCounter(), 1, MAX_STEPS);
    ImGui::PopItemWidth();
}

void DrumView::drawResetAllButton()
{
    if (ImGui::Button("Reset All"))
    {
        drum_controller_.resetAllTracks();
    }
}

void DrumView::drawBpmControls(int &bpm)
{

    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("BPM");
    ImGui::SameLine();

    ImGui::PushItemWidth(100.0f);
    if (ImGui::InputInt("##BPM", &bpm, 1, 10))
    {
        if (bpm < 20)
        {
            drum_controller_.setBpm(20);
        }
        else if (bpm > 999)
        {
            drum_controller_.setBpm(999);
        }
        else
        {
            drum_controller_.setBpm(bpm);
        }
    }
    ImGui::PopItemWidth();
    ImGui::EndGroup();
}

void DrumView::drawMasterVolume(float &volume)
{
    float scale = getScaleFactor();
    ImGui::Text("Master Volume");

    ImGui::PushItemWidth(std::clamp(200.0f * scale, 100.0f, 300.0f));
    drawCustomVolumeSlider("##Master_Volume", -1, volume, 0, 5);
    ImGui::PopItemWidth();
}

void DrumView::drawTogglePlayButton()
{
    float scale = getScaleFactor();

    ImGui::PushItemWidth(100.0f * scale);
    std::string playing_status = drum_controller_.getIsPlaying() ? "Pause" : "Play";
    if (ImGui::Button(playing_status.c_str()))
    {
        drum_controller_.toggleSequencer();
    }
    ImGui::PopItemWidth();
}

void DrumView::drawControls(float start_x)
{
    int bpm = drum_controller_.getBpm();
    float volume = drum_controller_.getMasterVolume();

    ImGui::SetCursorPosX(start_x);
    drawTogglePlayButton();

    float text_width = ImGui::CalcTextSize("BPM").x;
    float item_spacing = ImGui::GetStyle().ItemSpacing.x;
    float input_width = 100.0f;
    float total_group_size = text_width + item_spacing + input_width;

    float right_align = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x;

    ImGui::SameLine();
    ImGui::SetCursorPosX((right_align - total_group_size) + 5.0f);
    drawBpmControls(bpm);

    ImGui::NewLine();
    ImGui::SetCursorPosX(start_x);
    drawMasterVolume(volume);
    ImGui::NewLine();
}

void DrumView::drawDrumPackSelectionMenu()
{
    std::string curr_drum_pack = drum_controller_.extractDirName(drum_controller_.getCurrDrumPack());
    std::vector<std::string> drum_packs = drum_controller_.getDrumPacks();

    // extract drum pack names
    for (size_t i = 0; i < drum_packs.size(); i++)
    {
        std::string d_name = drum_controller_.extractDirName(drum_packs.at(i));
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
        if (p.has_extension())
        {
            auto extension = p.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

            // TODO:Test .wav,mp3,mp4,ogg
            if (extension == ".wav" || extension == ".mp3")
            {
                files_dropped_buf.push_back(std::move(p));
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
        for (auto &path : files_dropped_buf)
        {
            if (std::filesystem::exists(path))
            {
                float w = ImGui::CalcTextSize(path.string().c_str()).x;

                max_text_width = (std::max)(max_text_width, w);
                files_accepted.insert(std::move(path));
            }
        }

        files_dropped_buf.clear();

        if (files_accepted.size() > 0)
        {
            float padding_x = 15.0f;
            list_box_size.x = max_text_width + padding_x;
        }

        // TODO:Extract Later?
        if (ImGui::BeginListBox("##Dropped Samples", list_box_size))
        {
            std::set<std::filesystem::path>::iterator to_erase = files_accepted.end();

            int idx = 0;
            for (auto it = files_accepted.begin(); it != files_accepted.end(); ++it, ++idx)
            {
                const std::filesystem::path &file_path = *it;

                if (ImGui::Selectable(file_path.string().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        to_erase = it;
                    }
                }
            }

            if (to_erase != files_accepted.end())
            {
                files_accepted.erase(to_erase);
            }

            ImGui::EndListBox();
        }

        ImGui::BeginDisabled(files_accepted.empty());
        if (ImGui::Button("Submit"))
        {
            drum_controller_.startCopyQueue(files_accepted);
            files_accepted.clear();
            open_add_samples_modal_ = false;
            open_copy_progress_modal_ = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            files_accepted.clear();
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

void DrumView::drawDebug()
{
    float partition = getPartitionSize();

    ImGui::SetCursorPosY(25.0f);
    ImGui::SetCursorPosX(0);
    ImGui::Text("0");
    ImGui::SameLine();
    for (int i = 1; i < 8; i++)
    {
        ImGui::SetCursorPosX((i * partition));
        ImGui::Text(std::to_string(i).c_str());
        ImGui::SameLine();
    }
    ImU32 line_color = ImColor(255, 255, 255, 255);
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine(ImVec2((partition * 1), 50), ImVec2((partition * 7), 50), line_color, 1.0f);
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
