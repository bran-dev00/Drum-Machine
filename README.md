# Drum-Machine

An 8 track drum machine project written in C++, with miniaudio and dearimgui libraries

## Features
- 8 Tracks
- Import your own samples
- Save Custom Presets
- Create your own custom 8 track drum kits


# Build Commands

## Windows
- You can use the build_and_run.bat script or enter the following commands:
    - git submodule update --init --recursive
    - cmake -G "MinGW Makefiles" -S . -B build
    - cmake --build build
    - .\build\DrumMachine.exe

## Linux
- Similarly you can use the build_and_run.sh script or enter the following commands:
    - git submodule update --init --recursive
    - cmake -G "Unix Makefiles" -S . -B build
    - cmake --build build
    - ./build/DrumMachine.exe