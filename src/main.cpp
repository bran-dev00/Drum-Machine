#include <filesystem>
#include <stdio.h>
#include <iostream>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "drum_controller.hpp"
#include "drum_view.hpp"
#include "themes.hpp"

int main(int argc, char const *argv[])
{

    // imgui + glfw boilerplate
    if (!glfwInit())
        return 1;

    // Create window with OpenGL context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow((int)(1920 * main_scale), (int)(1080 * main_scale), "Drum Machine", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return 1;
    }

    // restrict min window size
    glfwSetWindowSizeLimits(window, 800, 800, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Theme Defaults
    ImGui::StyleColorsDark();

    // Custom Theme
    setCustomTheme();

    // Window Icon
    GLFWimage images[1];
    std::string icon_path = (std::filesystem::current_path() / L"assets" / L"images" / L"desktop_icon.png").string();
    images[0].pixels = stbi_load(icon_path.c_str(), &images[0].width, &images[0].height, 0, 4);

    glfwSetWindowIcon(window, 1, images);

    // Backend Bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Init Drum Machine
    auto drum_controller = new DrumController();
    auto drum_view = new DrumView(*drum_controller);

    // load main session

    drum_controller->loadSession();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();

        drum_controller->step();
        drum_view->draw();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.40f, 0.60, 0.47f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    Preset main_session_before_close = drum_view->savedCurrentPreset("Main_Session");
    drum_controller->saveSession(main_session_before_close);

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
