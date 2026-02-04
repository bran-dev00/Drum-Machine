#include <Windows.h>
#include <mmsystem.h>

#include <filesystem>
#include <stdio.h>
#include <iostream>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <GLFW/glfw3.h>

#include "drum_controller.hpp"

int main(int argc, char const *argv[])
{

// imgui + glfw boilerplate
    if (!glfwInit())
        return 1;

    // Create window with OpenGL context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Drum Machine", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Backend Bindings  
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();
    
    //Init Drum Machine
    auto sampleWav = (std::filesystem::current_path() / L"assets" / L"Rimshot.wav").wstring();
    auto drum_controller  = new DrumController(sampleWav); 
    
    //Test:
 /*    drum_controller->setSequencerNoteTrue(2);
    drum_controller->setSequencerNoteTrue(4);
    drum_controller->setSequencerNoteTrue(6);
    drum_controller->setSequencerNoteTrue(8) */;
    
    
    //time keeper
    std::chrono::time_point<std::chrono::steady_clock> last_sound_timepoint = std::chrono::steady_clock::now();


    int bpm = 120;
    bool isPlayingNow = false;
    int sequencerSize = drum_controller->getSequencerArray().size();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::ShowDemoWindow();

        // Current time point
        // auto sampleWav = (std::filesystem::current_path() / L"assets" / L"Snare.wav").wstring();
        auto now = std::chrono::steady_clock::now();
        // Apply updated BPM from the UI before advancing the sequencer step
        drum_controller->step();
        drum_controller->setBpm(bpm);

        
        //sequencer string representation

        {
            ImGui::Begin("Drum Machine");


            for(int i = 0; i< sequencerSize; i++){
                std::string id = "##beat" + std::to_string(i);
                
                bool * currBeat = &drum_controller->getSequencerArray().at(i);
                
                //Check if the checkbox is marked true and match those changes in the sequencerArray
                if(ImGui::Checkbox(id.c_str(), currBeat)){
                    if(drum_controller && i < MAX_STEPS){
                        if(*currBeat == true){
                            // std::cout << "True" << std::endl;
                            drum_controller->setSequencerNoteTrue(i);                    
                        }else{
                            // std::cout << "False" << std::endl;
                            drum_controller->setSequencerNoteFalse(i);
                        } 
                    }
                }

                ImGui::SameLine();
            }

            ImGui::NewLine();
            //sample path
            
            ImGui::PushItemWidth(100);
            if(ImGui::InputInt("BPM",&bpm, 1, 10)){
                
                
            }
            if(ImGui::Button("Play Sequencer")){
                drum_controller->playSequencer();
            }
            
            if(ImGui::Button("Stop Sequencer")){
                drum_controller->pauseSequencer();
            }

            if (ImGui::Button("Play Snare")) {
                PlaySoundW(sampleWav.c_str(), NULL, SND_FILENAME | SND_ASYNC);
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
