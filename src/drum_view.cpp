#include "drum_view.hpp"
#include "imgui.h"
#include <iostream>
#include <array>

DrumView::DrumView(DrumController &controller) : drum_controller_(controller)
{
}

DrumView::~DrumView() = default;

static void BeatCounterLabels(ImGuiStyle &styles, const std::array<float, MAX_STEPS> &positions)
{
    float checkbox_width = ImGui::GetFrameHeight();
    float y = ImGui::GetCursorScreenPos().y;

    for (size_t i = 0; i < positions.size(); ++i)
    {
        /*  // Beat 1,4,8,12,16
         if (i != 0 && i != 3 && i != 7 && i != 11 && i != 15)
         {
             continue;
         } */

        std::string label = std::to_string(static_cast<int>(i) + 1);
        ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
        float x_offset = checkbox_width * 0.5f;
        float x = positions[i] + (checkbox_width - text_size.x) / 2.0f + x_offset;
        ImGui::SetCursorScreenPos(ImVec2(x, y));
        ImGui::Text(label.c_str());
    }
}

void DrumView::draw()
{
    // Window
    const ImVec2 base_resolution = ImVec2(1920.0f, 1080.0f);
    ImVec2 current_size = ImGui::GetIO().DisplaySize;

    // Setup
    auto sample_wav = (std::filesystem::current_path() / L"assets" / L"Rimshot.wav").string();
    int bpm = drum_controller_.getBpm();
    float volume = drum_controller_.getVolume();

    // Styling
    ImGuiStyle &styles = ImGui::GetStyle();
    styles.FrameRounding = 3.0f;

    {
        // ImGui::SetNextWindowSize(drum_machine_window_size);
        ImGui::Begin("Drum View", NULL, ImGuiWindowFlags_NoResize);
        styles.WindowPadding = ImVec2(10.0f, 10.0f);
        ImVec2 window_size = ImGui::GetWindowSize();
        // std::cout << window_size.x << "," << window_size.y << "\n";

        // Width Offset
        // ImGui::SetCursorPosX(100.0f);
        ImGui::PushItemWidth(window_size.x);
        ImGui::SliderInt("##", &drum_controller_.getBeatCounter(), 1, MAX_STEPS);

        auto &tracks = drum_controller_.getTracks();
        int num_tracks = static_cast<int>(tracks.size());

        std::array<float, MAX_STEPS> checkbox_positions;
        // std::cout << "current_size: " << current_size.x << "," << current_size.y << "\n";
        for (int i = 0; i < num_tracks; ++i)
        {
            std::string track_name = tracks[i].getName();
            ImGui::SeparatorText(track_name.c_str());
            Track_t &track = tracks.at(i).getTrackSequencer();

            for (int j = 0; j < MAX_STEPS; ++j)
            {
                std::string id = std::string("##track_") + std::to_string(i) + "_beat_" + std::to_string(j);

                // Record checkbox X position (screen-space) before drawing
                float checkbox_x = ImGui::GetCursorScreenPos().x;
                checkbox_positions[j] = checkbox_x;

                // Checkbox Size
                styles.FramePadding = ImVec2(10.0f, 10.0f);
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
                ImGui::SameLine();
            }

            // Change Padding Back
            styles.FramePadding = ImVec2(4.0f, 3.0f);
            ImGui::SameLine();
            ImGui::PushID(i);

            if (ImGui::Button("Reset"))
            {
                drum_controller_.resetSequencer(track);
            }
            ImGui::PopID();
            ImGui::NewLine();
            // ImGui::NewLine();
        }
        BeatCounterLabels(styles, checkbox_positions);
        ImGui::NewLine();

        ImGui::PushItemWidth(100);

        if (ImGui::InputInt("BPM", &bpm, 1, 10))
        {
            drum_controller_.setBpm(bpm);
        }

        ImGui::NewLine();

        // Buttons
        std::string buttonDisplayStatus = drum_controller_.getIsPlaying() ? "Pause" : "Play";

        if (ImGui::Button(buttonDisplayStatus.c_str()))
        {
            drum_controller_.toggleSequencer();
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset All"))
        {
            drum_controller_.resetAllTracks();
        }

        if (ImGui::Button("Play Sample"))
        {
            drum_controller_.playSound(sample_wav);
        }

        // Volume
        if (ImGui::SliderFloat("Volume", &volume, 0, 5))
        {
            drum_controller_.setVolume(volume);
        }

        ImGui::End();
    }
}
