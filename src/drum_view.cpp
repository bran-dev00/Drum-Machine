#include "drum_view.hpp"
#include "imgui.h"

DrumView::DrumView(DrumController &controller) : drum_controller_(controller)
{
}

DrumView::~DrumView() = default;

void DrumView::draw()
{
    // Setup
    auto sampleWav = (std::filesystem::current_path() / L"assets" / L"Rimshot.wav").string();
    int bpm = drum_controller_.getBpm();
    float volume = drum_controller_.getVolume();

    {
        ImGui::Begin("Drum View");

        ImGui::PushItemWidth((16 * 16) * 2);
        ImGui::SliderInt("##", &drum_controller_.getBeatCounter(), 1, MAX_STEPS);
        auto &tracks = drum_controller_.getTracks();
        int num_tracks = static_cast<int>(tracks.size());

        for (int i = 0; i < num_tracks; ++i)
        {
            std::string track_name = tracks[i].getName();
            ImGui::SeparatorText(track_name.c_str());
            Track_t &track = tracks.at(i).getTrackSequencer();

            for (int j = 0; j < MAX_STEPS; ++j)
            {
                std::string id = std::string("##track_") + std::to_string(i) + "_beat_" + std::to_string(j);

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
            // Individual Reset Button
            ImGui::SameLine();
            ImGui::PushID(i);

            if (ImGui::Button("Reset"))
            {
                drum_controller_.resetSequencer(track);
            }
            ImGui::PopID();
            ImGui::NewLine();
            ImGui::NewLine();
        }
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
            drum_controller_.playSound(sampleWav);
        }

        // Volume
        if (ImGui::SliderFloat("Volume", &volume, 0, 5))
        {
            drum_controller_.setVolume(volume);
        }

        ImGui::End();
    }
}
