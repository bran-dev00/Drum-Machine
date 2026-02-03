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

add_executable(DrumMachine src/main.cpp)

# Build drum_controller as a static library and expose its headers
add_library(drum_controller STATIC src/drum_controller.cpp)
target_include_directories(drum_controller PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(DrumMachine PRIVATE drum_controller imgui glfw OpenGL::GL)

if(WIN32)
	# On Windows also link to winmm and ensure drum_controller is linked
	target_link_libraries(DrumMachine PRIVATE winmm drum_controller imgui glfw OpenGL::GL)
endif()