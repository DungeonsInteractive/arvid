/*Program to display simple messages and values on an LCD screen
Adapted from publically available GITHUB program for an arduino
01/11/23
Version 3
FURTHER MODIFIED BY SAM HOBERN
*/

/* Ensure RW LCD pin is connected to GND and connect a 10 K potentiometer with wiper connected to V0 and other ends to A and K LCD pins.
 Connect a 100 ohm resistor (ideally 500 Ohm) between A LCD pin and VDD.
*/

#include "mbed.h"
#include "TextLCD_CC.h"
#include "vector"
#include "map"
#include "string"

using namespace std;


InterruptIn rightButtonPress(), leftButtonPress();
Thread rightButton(), leftButton();
Semaphore buttonPressController();


void rBThreadHelper() {

    while (true) {

        buttonPressController().acquire();
        
    }
};



class Entity {

    private:

    int health, mana, atk, move_Speed;

    public:

    Entity() {

    };

    void hit(int dmg) {

        health -= dmg;
    };

    void heal(int hp_increase) {

        health += hp_increase;
    }

};

class Player : Entity {

    private:

    public:
};


class Enemy : Entity {
    private:

    public:

};


void characterSetup(TextLCD &lcd_) {
    //letter A capital
    char letterA[] =  { 0x0E, 0x1F, 0x1B, 0x1B, 0x1F, 0x1B, 0x1B, 0x1B };

    //letter R capital
    char letterR[] = { 0x1C, 0x1E, 0x1B, 0x19, 0x1F, 0x1B, 0x1B, 0x1B };

    //letter V capital
    char letterV[] = { 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x1F, 0x0E };

    //letter I capital
    char letterI[] = { 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F };

    //letter D capital
    char letterD[] = { 0x1E, 0x1F, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x1E };


    lcd_.writeCustomCharacter(letterA, 1);
    lcd_.writeCustomCharacter(letterR, 2);
    lcd_.writeCustomCharacter(letterV, 3);
    lcd_.writeCustomCharacter(letterI, 4);
    lcd_.writeCustomCharacter(letterD, 5);

}

void startup(TextLCD &lcd_) {
    lcd_.printf("    %c %c %c %c %c", 0, 1, 2, 3, 4);

}
void processStates() {

}
void updateStates() {

}
void gameLoop() {

}

int main() {
   

    //std::string customCharacterSet;
    //customCharacterSet.reserve(5);
    //customCharacterSet.emplace_back(letterA);

    TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD16x2);
    lcd.locate(0,0);

    characterSetup(lcd);

    while(true) {
        startup(lcd);
        thread_sleep_for(2000);
        lcd.cls();

    }
}

/*
custom Chars

Archer
byte customChar[] = {
  B01010,
  B00100,
  B01100,
  B10100,
  B10100,
  B01100,
  B00100,
  B01010
};


player
byte customChar[] = {
  B00000,
  B00100,
  B01010,
  B00100,
  B01110,
  B00100,
  B01010,
  B01010
};

slime
byte customChar[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00110,
  B00000,
  B00000
};

viking
byte customChar[] = {
  B00000,
  B01110,
  B01110,
  B10100,
  B11111,
  B10100,
  B01010,
  B01010
};

fighter
byte customChar[] = {
  B00000,
  B00100,
  B01110,
  B01110,
  B01110,
  B01110,
  B01010,
  B01010
};


letter A capital
byte customChar[] = {
  B01110,
  B11111,
  B11011,
  B11011,
  B11111,
  B11011,
  B11011,
  B11011
};

letter R capital
byte customChar[] = {
  B11100,
  B11110,
  B11011,
  B11001,
  B11111,
  B11011,
  B11011,
  B11011
};

letter V capital
byte customChar[] = {
  B11011,
  B11011,
  B11011,
  B11011,
  B11011,
  B11011,
  B11111,
  B01110
};

letter I capital
byte customChar[] = {
  B11111,
  B11111,
  B01110,
  B01110,
  B01110,
  B01110,
  B11111,
  B11111
};

letter D capital
byte customChar[] = {
  B11110,
  B11111,
  B11011,
  B11011,
  B11011,
  B11011,
  B11111,
  B11110
};


*/