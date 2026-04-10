#pragma once
#include "mbed.h"
#include <array>

class Joystick {
    private:

    AnalogIn analogX_, analogY_;

    public:

        Joystick(PinName analogX, PinName analogY);

        array<float, 2> reportPosition();

};