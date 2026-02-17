#include "drum_view.hpp"
#include <string>

DrumView::DrumView(DrumController &controller) : drum_controller_(controller)
{
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
    for (int j = 0; j < MAX_STEPS; ++j)
    {

        std::string id = std::string("##track_") + std::to_string(track_idx) + "_beat_" + std::to_string(j);

        // Checkbox Size
        styles_.FramePadding = ImVec2(10.0f, 10.0f);
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
    styles_.FrameRounding = 3.0f;

    {
        ImGui::Begin("Drum View", NULL, ImGuiWindowFlags_NoResize);
        styles_.WindowPadding = ImVec2(10.0f, 10.0f);
        ImVec2 window_size = ImGui::GetWindowSize();

        // Beat Indicator
        ImGui::SetCursorPosX(10.0f);
        float checkbox_width = ImGui::GetFrameHeight() * 2;
        ImGui::PushItemWidth((checkbox_width * MAX_STEPS) + 40.0f);
        ImGui::SliderInt("##", &drum_controller_.getBeatCounter(), 1, MAX_STEPS);

        auto &tracks = drum_controller_.getTracks();
        int num_tracks = static_cast<int>(tracks.size());

        // Store the checkbox positions to calculate label position
        std::array<float, MAX_STEPS> checkbox_positions{};

        for (int i = 0; i < num_tracks; ++i)
        {
            std::string track_name = tracks[i].getName();
            ImGui::SeparatorText(track_name.c_str());
            Track_t &track = tracks.at(i).getTrackSequencer();

            // Change Padding Back
            // styles_.FramePadding = ImVec2(4.0f, 3.0f);
            ImGui::PushID(i);

            drawTrack(i, track, checkbox_positions);

            // Restore smaller padding for buttons and other widgets
            styles_.FramePadding = ImVec2(4.0f, 3.0f);

            if (ImGui::Button("Reset"))
            {
                drum_controller_.resetSequencer(track);
            }
            ImGui::PopID();
            ImGui::NewLine();
        }

        drawBeatCounterLabels(checkbox_positions);
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
