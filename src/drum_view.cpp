#include "drum_view.hpp"
#include "imgui.h"

DrumView::DrumView(DrumController &controller) : controller_(controller)
{
}

void DrumView::draw()
{
    {
        ImGui::Begin("Drum View");

        ImGui::Text("Hello World");

        ImGui::End();
    }
}