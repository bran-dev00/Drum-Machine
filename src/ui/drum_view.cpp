#include "drum_view.hpp"
#include <imgui_internal.h>

DrumView::DrumView(DrumController &controller)
    : drum_controller_(controller),
      drum_view_controls_(controller),
      drum_view_tracks_(controller),
      drum_view_menu_(controller)
{
    ImGuiIO &io = ImGui::GetIO();

    float scale = getScaleFactor();
    float font_size = std::clamp(16.0f * scale, 16.0f, 32.0f);

    std::string lexend_font = (std::filesystem::current_path() / L"fonts" / L"Lexend-Medium.ttf").string();
    io.Fonts->AddFontFromFileTTF(lexend_font.c_str(), font_size);
}

DrumView::~DrumView() = default;

void DrumView::onFilesDropped(int count, const char **paths)
{
    drum_view_menu_.onFilesDropped(count, paths);
}

Preset DrumView::savedCurrentPreset(std::string preset_name)
{
    return drum_view_menu_.savedCurrentPreset(preset_name);
}

void DrumView::drawHoverCursor()
{
    ImU32 color_fill = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 color_border = ImGui::GetColorU32(ImVec4(255.0f, 255.0f, 255.0f, 1.0f));

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

void DrumView::drawMainContainer(float start_x, float width)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 3.0f);

    ImGui::SetCursorPosX(start_x);
    ImGui::BeginChild("##MainContainer", ImVec2(width, 0), false, ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeY);

    drawControls(0);
    drum_view_tracks_.drawTracks(width);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void DrumView::draw()
{
    ImVec2 display = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(display, ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("Drum Machine", NULL, window_flags);

    drum_view_menu_.drawMenuBar();

    // Handle keyboard shortcuts
    if (ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        drum_controller_.toggleSequencer();
    }

    // DrumViewUtils::drawDebug();

    float partition_size = getPartitionSize();

    float start_x = partition_size;
    float end_x = partition_size;

    // start_x is the left edge of the main container
    // end_x is the how many partition sizes the main container spans
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
