#include "mbed.h"
#include "Joystick.h"

Joystick::Joystick(PinName analogX, PinName analogY) : analogX_(analogX), analogY_(analogY) {
    printf("New object created");
}


void Joystick::reportPosition() {

    float posX{0}, posY{0};
    printf("I was called");
    posX = analogX_.read();
    posY = analogY_.read();
    printf("%.3f ", posX);
    printf("%.3f \n", posY);
}