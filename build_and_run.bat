git submodule update --recursive --init
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build
build\DrumMachine.exe