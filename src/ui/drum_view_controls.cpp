#include "ui/drum_view_controls.hpp"

void DrumViewControls::drawControls(float start_x)
{
    int bpm = drum_controller_.getBpm();
    float volume = drum_controller_.getMasterVolume();
    float scale = DrumViewUtils::getScaleFactor(ImGui::GetWindowSize().x);

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

void DrumViewControls::drawBpmControls(int &bpm)
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

void DrumViewControls::drawMasterVolume(float &volume)
{
    float scale = DrumViewUtils::getScaleFactor(ImGui::GetWindowSize().x);
    ImGui::Text("Master Volume");

    ImGui::PushItemWidth(std::clamp(120.0f * scale, 100.0f, 200.0f));
    DrumViewUtils::drawCustomVolumeSlider("##Master_Volume", -1, volume, -40, 10, drum_controller_);
    ImGui::PopItemWidth();
}

void DrumViewControls::drawTogglePlayButton()
{
    float scale = DrumViewUtils::getScaleFactor(ImGui::GetWindowSize().x);

    ImGui::PushItemWidth(100.0f * scale);
    std::string playing_status = drum_controller_.getIsPlaying() ? "Pause" : "Play";
    if (ImGui::Button(playing_status.c_str()))
    {
        drum_controller_.toggleSequencer();
    }
    ImGui::PopItemWidth();
}