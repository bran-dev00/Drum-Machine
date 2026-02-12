#pragma once

#include "drum_controller.hpp"
#include <filesystem>

class DrumView 
{
private:
    DrumController &drum_controller_;
public:
    DrumView(DrumController &controller);
    ~DrumView();
    
    void draw();
    
};
