#include "ui/drum_view_tracks.hpp"

void DrumViewTracks::drawResetAllButton()
{
    ImGui::PushStyleColor(ImGuiCol_Text, DrumViewUtils::BUTTON_TEXT_COLOR);
    if (ImGui::Button("Reset All"))
    {
        drum_controller_.resetAllTracks();
    }
    ImGui::PopStyleColor();
}

void DrumViewTracks::drawBeatIndicator(float width)
{
    float checkbox_width = ImGui::GetFrameHeightWithSpacing() * 1.5f;
    ImGui::PushItemWidth(width);
    ImGui::SliderInt("##Beat", &drum_controller_.getBeatCounter(), 1, MAX_STEPS);
    ImGui::PopItemWidth();
}

void DrumViewTracks::drawBeatCounterLabels(const std::array<float, MAX_STEPS> &positions)
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

void DrumViewTracks::drawTrack(int track_idx, Track_t &track, std::array<float, MAX_STEPS> &checkbox_positions, float checkbox_size)
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

void DrumViewTracks::drawTracks(float width)
{
    auto &tracks = drum_controller_.getTracks();
    std::array<float, NUM_TRACKS> track_volumes = drum_controller_.getTrackVolumes();

    // Store the checkbox positions to calculate label position
    std::array<float, MAX_STEPS> checkbox_positions{};

    float scale = DrumViewUtils::getScaleFactor(ImGui::GetWindowSize().x);
    float partition_size = DrumViewUtils::getPartitionSize();
    float item_spacing = ImGui::GetStyle().ItemSpacing.x;
    float font_size = ImGui::GetFontSize();

    float checkbox_budget = width * (4.0f / 5.0f);
    float checkbox_size = (std::max)(1.0f, (checkbox_budget / MAX_STEPS - font_size - item_spacing) * 0.5f);

    drawBeatIndicator(width);

    ImGui::BeginChild("Tracks", ImVec2(width, 0), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

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
        ImGui::PushStyleColor(ImGuiCol_Text, DrumViewUtils::BUTTON_TEXT_COLOR);
        if (ImGui::Button("Reset"))
        {
            drum_controller_.resetSequencer(track);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();

        float button_height = ImGui::GetFrameHeight();
        float slider_height = button_height / 2;
        float vertical_offset = (button_height - slider_height) * 0.5f;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + vertical_offset + checkbox_size);

        // volume slider width
        ImGui::PushItemWidth((std::max)(100.0f * scale, 150.0f));
        DrumViewUtils::drawCustomVolumeSlider("Volume", i, track_volumes.at(i), -40, 10, drum_controller_);
        ImGui::PopItemWidth();

        ImGui::PopID();

        ImGui::NewLine();
    }

    drawBeatCounterLabels(checkbox_positions);

    ImGui::SameLine(0.0f, (checkbox_size + item_spacing + 1.0f));
    drawResetAllButton();
    ImGui::EndChild();
}