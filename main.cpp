#include <Windows.h>
#include <iostream>
#include <mmsystem.h>
#include <chrono>
using namespace std::chrono_literals;

int main(int argc, char const *argv[])
{
    
    std::chrono::time_point<std::chrono::steady_clock> last_sound_timepoint = std::chrono::steady_clock::now();

    while(true){
        auto now = std::chrono::steady_clock::now();

        if(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_sound_timepoint) > 1000ms ){
            PlaySound(TEXT("Snare.wav"), NULL, SND_FILENAME | SND_ASYNC);
            last_sound_timepoint = now;
        }
    }

    return 0;

}
