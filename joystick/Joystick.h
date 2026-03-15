#pragma once
#include "mbed.h"

class Joystick {
    private:

    AnalogIn analogX_, analogY_;

    public:

        Joystick(PinName analogX, PinName analogY);

        void reportPosition();
};