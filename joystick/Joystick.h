#pragma once
#include "mbed.h"
#include <array>

class Joystick {
    private:

    AnalogIn analogX_, analogY_;
    DigitalIn pushB_;

    public:

        Joystick(PinName analogX, PinName analogY, PinName pushB);

        array<float, 2> reportPosition();

        bool reportPush();

};