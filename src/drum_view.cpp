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
        styles_.FramePadding = ImVec2(5.0f, 5.0f);
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

void DrumView::drawSubMenu()
{
    int bpm = drum_controller_.getBpm();
    float volume = drum_controller_.getVolume();
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
        std::string buttonDisplayStatus = drum_controller_.getIsPlaying() ? "Pause" : "Play";
        if (ImGui::Button(buttonDisplayStatus.c_str()))
        {
            drum_controller_.toggleSequencer();
        }

        if (ImGui::Button("Reset All"))
        {
            drum_controller_.resetAllTracks();
        }
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Volume");
        ImGui::SameLine();

        ImGui::PushItemWidth(100.0f);
        if (ImGui::SliderFloat("##Volume", &volume, 0, 5))
        {
            drum_controller_.setVolume(volume);
        }
        ImGui::EndChild();
    }
}

void DrumView::draw()
{
    // Window
    ImVec2 current_size = ImGui::GetIO().DisplaySize;

    // Styling
    styles_.FrameRounding = 3.0f;
    {
        ImGui::Begin("Drum View", NULL);
        ImVec2 window_size = ImGui::GetWindowSize();

        // Center Align
        styles_.WindowPadding = ImVec2(window_size.x / 5.0f, 10.0f);
        drawSubMenu();

        ImGui::SetCursorPosX(window_size.x / 5.0f);
        float checkbox_width = ImGui::GetFrameHeightWithSpacing() * 1.5f;
        ImGui::PushItemWidth((checkbox_width * MAX_STEPS));
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

            ImGui::PushID(i);
            drawTrack(i, track, checkbox_positions);

            if (ImGui::Button("Reset"))
            {
                drum_controller_.resetSequencer(track);
            }
            styles_.FramePadding = ImVec2(4.0f, 3.0f);
            ImGui::PopID();
            ImGui::NewLine();
        }

        drawBeatCounterLabels(checkbox_positions);
        ImGui::NewLine();
        ImGui::PushItemWidth(100);
        ImGui::End();
    }
}
