#!/bin/bash
git submodule update --init --recursive
cmake -G "Unix Makefiles" -S . -B build
cmake --build build
./build/DrumMachine