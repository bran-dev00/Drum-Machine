#pragma once
#include "imgui.h"
#include "drum_controller.hpp"
#include "imgui_internal.h"

namespace DrumViewUtils
{
    inline float getScaleFactor(const float base_width)
    {
        return ImGui::GetIO().DisplaySize.x / base_width;
    }

    inline float getPartitionSize()
    {
        return ImGui::GetIO().DisplaySize.x / 8.0f;
    }

    inline void drawCustomVolumeSlider(std::string label, int track_idx, float &value, float v_min, float v_max, DrumController &drum_controller)
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
                drum_controller.setMasterVolume(value);
            }
            else
            {
                drum_controller.setSoundVolume(track_idx, value);
            }
        }

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bg_color, height * 0.5f);
        float fill_width = ((value - v_min) / (v_max - v_min)) * width;
        draw_list->AddRectFilled(p, ImVec2(p.x + fill_width, p.y + height), handle_color, height * 0.5f);

        float handle_x = p.x + fill_width;
        draw_list->AddCircleFilled(ImVec2(handle_x, p.y + height * 0.5f), height * 0.75f, handle_color);
    }

    inline void drawDebug()
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

}