// Arvid pre-release

#include "Joystick.h"
#include "TextLCD_CC.h"
#include "mbed.h"
#include <cstdint>
#include <cstring>

using namespace std;
using Entity = uint8_t;

// Initialise main and side LCDs using morpho and digital pins
TextLCD mainLCD(PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, TextLCD::LCD20x4);
TextLCD sideLCD(D0, D1, D2, D3, D4, D5, TextLCD::LCD16x2);

// Joystick for controlling directional movement
Joystick mainJoystick(A5, A4);

// Buttons for user input
DigitalIn selectorButton(D10);
DigitalIn menuButton(D11);




// store game states for future stuff

enum GAMESTATE {
    MENU,
    CHARACTER_SELECT,
    LOADING_SCREEN,
    END_SCREEN,
    MAP,
    FIGHT
};

enum CLASS { 
    BERSERKER,
    MAGE,
    RANGER,
    TANK
};


// Implementing the ECS model

// Entities

const uint16_t maxEntities = 16;
Entity nextEntityID = 0;
CLASS playClass;



// Components
// Boolean arrays keep track of what entities have what components


struct Position {
  uint16_t x;
  uint16_t y;  
};
Position positionComponent[maxEntities];
bool hasPosition[maxEntities];

struct Velocity {
    uint16_t changeX;
    uint16_t changeY;
};
Velocity velocityComponent[maxEntities];
bool hasVelocity[maxEntities];


struct Icons { 
    char symbol[8] = {};
};
Icons iconComponent[maxEntities];
bool hasIcon[maxEntities];

/*
if (classNumber == 0) {
        playClass = BERSERKER;
    } else if (classNumber == 1) {
        playClass = MAGE;
    } else if (classNumber == 1) {
        playClass = RANGER;
    } else if (classNumber == 1) {
        playClass = TANK;
    }*/

// Systems

// initialization system

char playerA[] = {0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F};

void setIconSymbols(Entity entity_, const char* symbol_) { 
    memcpy(iconComponent[entity_].symbol, symbol_, 8);
}

// helper function to initialise new entities

void initializeEntities(Entity id, uint16_t positionX_, uint16_t positionY_, uint16_t velocityX_, uint16_t velocityY_,  const char * symbol_) { 
    
    // this is where we set all the entitiy attributes

    positionComponent[id].x = positionX_;
    positionComponent[id].y = positionY_;
    
    velocityComponent[id].changeX = velocityX_;
    velocityComponent[id].changeY = velocityY_;

    setIconSymbols(id, symbol_);

}


// lets get moving
void renderSystemMain() {

    mainLCD.cls();
    for ( int i = 0; i < maxEntities; i++ ) {
        
        if (iconComponent[i].symbol[0] != 0) {
            mainLCD.locate(positionComponent[i].x, positionComponent[i].y);
            mainLCD.write(iconComponent[i].symbol, i);
            mainLCD.printf("%c", i);
        };
    }
}

void renderSystemSide() {

    sideLCD.cls();
    for ( int i = 0; i < maxEntities; i++ ) {
        
        if (iconComponent[i].symbol[0] != 0) {
            sideLCD.locate(positionComponent[i].x, positionComponent[i].y);
            sideLCD.writeCustomCharacter(iconComponent[i].symbol, i);
            sideLCD.printf("%c", 0);
        }
    }
}

void movementSystem() {

    for (int i = 0; i < maxEntities; ++i) {

        //if (!hasPosition[i]) continue;
        
        positionComponent[i].x += velocityComponent[i].changeX;
        positionComponent[i].y += velocityComponent[i].changeY;

    }
}

void inputSystem() {
    
    // Joystick input
    if (mainJoystick.reportPosition().empty()) return;
    
    float pos[2];
    pos[0] = mainJoystick.reportPosition()[0];
    pos[1] = mainJoystick.reportPosition()[1]; 

    // need to redo the mapping i.e 700 - 1000 is what direction etcetc
    if (pos[0] > 700) {

    }
    
    // Button inputs
    // button for select, button for menu -- will think of more

    if (menuButton) { 
        //
    }
   

}


void systemManager() { 
    // to be completed z
}



int main() {

    //Initiialize Joystick
    initializeEntities(0, 0,0, 1, 0, playerA);
    
    
    while (true) {
        renderSystemSide();

        movementSystem();
        thread_sleep_for(500);
        
    }

}
