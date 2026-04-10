#include "mbed.h"
#include "Joystick.h"

Joystick::Joystick(PinName analogX, PinName analogY) : analogX_(analogX), analogY_(analogY) { };

array<float, 2> Joystick::reportPosition() {

    float posX{0}, posY{0};

    posX = analogX_.read();
    posY = analogY_.read();
    
    array<float, 2> returnVal;
    returnVal[0] = posX;
    returnVal[1] = posY;

    return returnVal;   
}

