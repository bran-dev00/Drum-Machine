#pragma once
#include "imgui.h"

static void setCustomTheme()
{

    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    ImColor check_mark_color = ImColor(240, 240, 255, 255);
    ImColor window_bg_color = ImColor(26, 26, 46, 255);

    ImColor frame_bg_color = ImColor(40, 40, 75, 255);
    ImColor frame_bg_hovered_color = ImColor(80, 80, 110, 255);
    ImColor frame_bg_active_color = ImColor(80, 80, 140, 255);

    ImColor button_color = ImColor(168, 167, 255, 255);
    ImColor button_hovered_color = ImColor(200, 199, 255, 255);
    ImColor button_active_color = ImColor(124, 124, 204, 255);

    ImColor text_color = ImColor(240, 240, 255, 255);
    ImColor slider_grab_color = ImColor(163, 163, 255, 255);

    ImColor menu_bar_bg_color = ImColor(32, 32, 64, 255);
    ImColor popup_bg_color = ImColor(35, 35, 70, 255);
    ImColor header_color = ImColor(80, 80, 140, 255);
    ImColor header_hovered_color = ImColor(107, 107, 204, 255);
    ImColor header_active_color = ImColor(137, 137, 255, 255);
    ImColor modal_bg_color = ImColor(0, 0, 0, 200);
    ImColor menu_item_hovered_color = ImColor(100, 100, 180, 255);

    colors[ImGuiCol_WindowBg] = window_bg_color;
    colors[ImGuiCol_FrameBg] = frame_bg_color;
    colors[ImGuiCol_FrameBgHovered] = frame_bg_hovered_color;
    colors[ImGuiCol_FrameBgActive] = frame_bg_active_color;

    colors[ImGuiCol_Button] = button_color;
    colors[ImGuiCol_ButtonHovered] = button_hovered_color;
    colors[ImGuiCol_ButtonActive] = button_active_color;

    colors[ImGuiCol_CheckMark] = check_mark_color;
    colors[ImGuiCol_SliderGrab] = slider_grab_color;

    colors[ImGuiCol_Text] = text_color;
    colors[ImGuiCol_MenuBarBg] = menu_bar_bg_color;
    colors[ImGuiCol_TitleBg] = menu_bar_bg_color;
    colors[ImGuiCol_TitleBgActive] = menu_bar_bg_color;
    colors[ImGuiCol_PopupBg] = popup_bg_color;
    colors[ImGuiCol_FrameBg] = frame_bg_color;
    colors[ImGuiCol_DragDropTarget] = menu_item_hovered_color;

    colors[ImGuiCol_Header] = header_color;
    colors[ImGuiCol_HeaderHovered] = header_hovered_color;
    colors[ImGuiCol_HeaderActive] = header_active_color;
    colors[ImGuiCol_ModalWindowDimBg] = modal_bg_color;
    colors[ImGuiCol_HeaderHovered] = menu_item_hovered_color;

    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
}