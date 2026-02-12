#pragma once

#include "drum_controller.hpp"

class DrumView 
{
private:
    DrumController controller_;
public:
    DrumView(DrumController &controller);
    ~DrumView();
    
    void draw();
};
