# Drum-Machine

An 8 track drum machine project written in C++ with the miniaudio and dearimgui libraries

<img width="1919" height="1047" alt="drum_machine" src="https://github.com/user-attachments/assets/b67fe759-3571-4449-a4d1-9a06a9c9d82b" />

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
