#include "drum_view.hpp"
#include <imgui_internal.h>

DrumView::DrumView(DrumController &controller) : drum_controller_(controller)
{
    // Global Styles
    styles_.FrameRounding = 3.0f;
    ImGuiIO &io = ImGui::GetIO();

    // Scale
    float scale = getScaleFactor();
    float font_size = std::clamp(16.0f * scale, 16.0f, 32.0f); // Clamp font size between 12 and 24

    std::string lexend_font = (std::filesystem::current_path() / L"fonts" / L"Lexend-Medium.ttf").string();
    io.Fonts->AddFontFromFileTTF(lexend_font.c_str(), font_size);
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

void DrumView::drawFileMenu()
{
    if (ImGui::Button("Save Session"))
    {
        drum_controller_.saveSession();
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
            drum_controller_.loadPreset(i);
        }
    }
}

void DrumView::drawSavePresetPopup()
{
    if (ImGui::BeginPopup("SavePresetPopup"))
    {
        static char preset_name[64] = "New Preset";

        ImGui::InputText("Preset Name", preset_name, sizeof(preset_name));

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
    bool open_delete_menu = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("File", NULL, false, false); // Non-interactive header
            drawFileMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Drum Packs"))
        {
            ImGui::MenuItem("Drum Packs", NULL, false, false); // Non-interactive header
            drawDrumPackSelectionMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Presets"))
        {
            ImGui::MenuItem("Presets", NULL, false, false); // Non-interactive header
            drawPresetsMenu();
            ImGui::NewLine();
            ImGui::Separator();

            if (ImGui::MenuItem("Save Current as Preset", NULL, false, true))
            {
                open_save_popup = true;
            }

            if (ImGui::BeginMenu("Remove Presets"))
            {
                ImGui::MenuItem("Remove Presets", NULL, false, false); // Non-interactive header
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

    drawSavePresetPopup();
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
