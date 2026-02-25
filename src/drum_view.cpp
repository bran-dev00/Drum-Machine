#include "drum_view.hpp"
#include <string>
#include <iostream>

DrumView::DrumView(DrumController &controller) : drum_controller_(controller)
{
    // Global Styles
    styles_.FrameRounding = 3.0f;
}

DrumView::~DrumView() = default;

void DrumView::drawBeatCounterLabels(const std::array<float, MAX_STEPS> &positions)
{
    float y = ImGui::GetCursorScreenPos().y;

    for (size_t i = 0; i < positions.size(); ++i)
    {
        std::string label = std::to_string(static_cast<int>(i) + 1);
        ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
        // positions contains the center X of each checkbox
        float x = positions[i] - (text_size.x / 2.0f);
        ImGui::SetCursorScreenPos(ImVec2(x, y));
        ImGui::TextUnformatted(label.c_str());
    }
}

void DrumView::drawTrack(int track_idx, Track_t &track, std::array<float, MAX_STEPS> &checkbox_positions)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
    for (int j = 0; j < MAX_STEPS; ++j)
    {

        std::string id = std::string("##track_") + std::to_string(track_idx) + "_beat_" + std::to_string(j);

        // Checkbox Size

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

void DrumView::drawTracks()
{

    auto &tracks = drum_controller_.getTracks();
    std::array<float, NUM_TRACKS> track_volumes = drum_controller_.getTrackVolumes();

    // Store the checkbox positions to calculate label position
    std::array<float, MAX_STEPS> checkbox_positions{};

    for (int i = 0; i < NUM_TRACKS; ++i)
    {
        std::string track_name = tracks[i].getName();
        ImGui::SeparatorText(track_name.c_str());
        Track_t &track = tracks.at(i).getTrackSequencer();
        ma_sound *sound = drum_controller_.getSound(i);

        ImGui::PushID(i);
        drawTrack(i, track, checkbox_positions);

        if (ImGui::Button("Reset"))
        {
            drum_controller_.resetSequencer(track);
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        {
            float track_volume = track_volumes.at(i);
            if (ImGui::SliderFloat("##track_vol", &track_volume, 0, 1))
            {
                drum_controller_.setSoundVolume(i, track_volume);
            }
        }
        ImGui::PopItemWidth();

        ImGui::PopID();
        ImGui::NewLine();
    }
    drawBeatCounterLabels(checkbox_positions);
}

void DrumView::drawBeatIndicator(ImVec2 window_size)
{
    ImGui::SetCursorPosX(window_size.x / 5.0f);
    float checkbox_width = ImGui::GetFrameHeightWithSpacing() * 1.5f;
    ImGui::PushItemWidth((checkbox_width * MAX_STEPS));
    ImGui::SliderInt("##Beat", &drum_controller_.getBeatCounter(), 1, MAX_STEPS);
    ImGui::PopItemWidth();
}

void DrumView::drawControls()
{
    int bpm = drum_controller_.getBpm();
    float volume = drum_controller_.getMasterVolume();
    auto sample_wav = (std::filesystem::current_path() / L"assets" / L"Rimshot.wav").string();

    auto window_size = ImGui::GetWindowSize();
    {
        ImGui::BeginChild("Basic Controls", ImVec2(window_size.x / 4, window_size.y / 5));

        ImGui::AlignTextToFramePadding();
        ImGui::Text("BPM");
        ImGui::SameLine();
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

        ImGui::NewLine();

        // Buttons
        ImGui::NewLine();
        // test
        std::string buttonDisplayStatus = drum_controller_.getIsPlaying() ? "Pause" : "Play";
        if (ImGui::Button(buttonDisplayStatus.c_str()))
        {
            drum_controller_.toggleSequencer();
        }

        ImGui::AlignTextToFramePadding();
        if (ImGui::Button("Reset All"))
        {
            drum_controller_.resetAllTracks();
        }

        ImGui::Text("Volume");
        ImGui::SameLine();

        ImGui::PushItemWidth(100.0f);
        if (ImGui::SliderFloat("##Volume", &volume, 0, 5))
        {
            drum_controller_.setMasterVolume(volume);
        }
        ImGui::PopItemWidth();
        ImGui::EndChild();
    }
}

void DrumView::drawDrumPackSelection()
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

void DrumView::drawMenu()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Drum Packs"))
        {
            drawDrumPackSelection();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

static void drawMainContainer()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ImGui::GetWindowSize().x * 0.5f, 5.0f));
    ImGui::BeginChild("##MainContainer");

    ImGui::Text("CONTENT GOES HERE");

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void DrumView::draw()
{
    // Window
    ImVec2 current_size = ImGui::GetIO().DisplaySize;
    {
        ImGui::Begin("Drum View", NULL, ImGuiWindowFlags_MenuBar);
        ImVec2 window_size = ImGui::GetWindowSize();

        drawMenu();
        drawControls();
        // drawMainContainer();
        drawBeatIndicator(window_size);

        // Center Align
        // styles_.WindowPadding = ImVec2(window_size.x / 8.0f, 10.0f);

        drawTracks();

        ImGui::NewLine();
        ImGui::End();
    }
}
