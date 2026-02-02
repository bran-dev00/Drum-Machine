cmake_minimum_required(VERSION 3.10)
project(DrumMachine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# Build GLFW
add_subdirectory(external/glfw-3.4)

# ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui-1.92.5)
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
)

target_link_libraries(DrumMachine PRIVATE imgui glfw OpenGL::GL)

if(WIN32)
	target_link_libraries(DrumMachine PRIVATE winmm imgui glfw OpenGL::GL)
endif()