cmake_minimum_required(VERSION 3.10)
project(DrumMachine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# Build GLFW
add_subdirectory(external/glfw)

# ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_BACKENDS ${IMGUI_DIR}/backends)

add_library(imgui STATIC
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_BACKENDS}/imgui_impl_glfw.cpp
	${IMGUI_BACKENDS}/imgui_impl_opengl2.cpp
)

target_include_directories(imgui PUBLIC
	${IMGUI_DIR}
	${IMGUI_BACKENDS}
)

# imgui needs GLFW headers and OpenGL for its backends
target_link_libraries(imgui PRIVATE glfw OpenGL::GL)

add_executable(DrumMachine 
	src/main.cpp 
	src/ui/drum_view.cpp
	src/ui/drum_view_controls.cpp
	src/ui/drum_view_tracks.cpp
	src/ui/drum_view_menu.cpp
	src/file_manager.cpp
	external/miniaudio/miniaudio.cpp
)

# Build drum_controller as a static library and expose its headers
add_library(drum_controller STATIC
	src/drum_controller.cpp
	src/drum_track_model.cpp
	src/presets.cpp
	src/drum_packs_manager.cpp
	src/drum_pack.cpp
	src/utils.cpp
)

target_include_directories(drum_controller PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_include_directories(DrumMachine PRIVATE ${CMAKE_SOURCE_DIR}/src)

#stb
target_include_directories(DrumMachine PRIVATE ${CMAKE_SOURCE_DIR}/external/stb_image)

#json
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_SOURCE_DIR}/external)
target_include_directories(DrumMachine PRIVATE ${CMAKE_SOURCE_DIR}/external)


target_link_libraries(drum_controller PUBLIC nlohmann_json)
target_link_libraries(DrumMachine PRIVATE drum_controller imgui glfw OpenGL::GL nlohmann_json)

if(WIN32)
	target_link_libraries(DrumMachine PRIVATE winmm)
endif()