// Arvid pre-release

#include "Joystick.h"
#include "TextLCD_CC.h"
#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace std;
using Entity = uint8_t;

// Initialise main and side LCDs using morpho and digital pins
TextLCD sideLCD(PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, TextLCD::LCD16x2);
TextLCD mainLCD(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4);

// Joystick for controlling directional movement
Joystick mainJoystick(A5, A4);

// Buttons for user input
DigitalIn selectorButton(D10);
DigitalIn menuButton(D11);

int REFRESH_RATE;
bool drawn = false;

struct  {

    // game symbols

    char symbolA[8] = { 0x0E, 0x1F, 0x1B, 0x1B, 0x1F, 0x1B, 0x1B, 0x1B };
    char symbolR[8] = { 0x1C, 0x1E, 0x1B, 0x19, 0x1F, 0x1B, 0x1B, 0x1B };
    char symbolV[8] = { 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x0E };
    char symbolI[8] = { 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F };
    char symbolD[8] = { 0x1E, 0x1F, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x1E };

    // selector icon
    char symbolSelector[8] = { 0x00, 0x00, 0x00, 0x1F, 0x0E, 0x04, 0x00, 0x00 };


    // class icons
    char fighter[8] = { 0x00, 0x00, 0x1F, 0x1F, 0x15, 0x1B, 0x11, 0x00 };
    char ranger[8] = { 0x10, 0x08, 0x04, 0x06, 0x06, 0x04, 0x08, 0x10 };
    char mage[8] = { 0x00, 0x04, 0x0E, 0x0E, 0x1F, 0x0E, 0x0A, 0x0E };
    char swordman[8] = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x04, 0x00 };
    char tank[8] = { 0x00, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00 };

} CharacterSymbols;


struct {

    // to be filled later 
} GameMaps;

enum J_DIRECTIONS{
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NEUTRAL
};


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
GAMESTATE currentGameState;
J_DIRECTIONS currentDirection;

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
void renderSystemSide() {

    mainLCD.cls();
    for ( int i = 0; i < maxEntities; i++ ) {
        
        if (iconComponent[i].symbol[0] != 0) {
            mainLCD.locate(positionComponent[i].x, positionComponent[i].y);
            mainLCD.write(iconComponent[i].symbol, i);
            mainLCD.printf("%c", i);
        };
    }
}

void renderSystemMain(GAMESTATE &currentState_) {

    // loading screen works
    if (currentState_ == LOADING_SCREEN) {
        mainLCD.cls();

        mainLCD.locate(5, 0);
        mainLCD.printf("Welcome to");
        mainLCD.locate(8, 1);
        
        // remember writeCustomCharacter is not 0 indexed its 1 indexed
        mainLCD.writeCustomCharacter(CharacterSymbols.symbolA, 1);
        mainLCD.writeCustomCharacter(CharacterSymbols.symbolR, 2);
        mainLCD.writeCustomCharacter(CharacterSymbols.symbolV, 3);
        mainLCD.writeCustomCharacter(CharacterSymbols.symbolI, 4);
        mainLCD.writeCustomCharacter(CharacterSymbols.symbolD, 5);
        
        // printf is annoyingly 0 indexed
        mainLCD.printf("%c%c%c%c%c", 0, 1, 2, 3, 4);    

        mainLCD.locate(2, 2);
        mainLCD.printf("Press any button ");
        mainLCD.locate(5, 3);
        mainLCD.printf("to start..");


        // place holder for now - should wait for button press to start

        thread_sleep_for(3000);

        currentState_ = CHARACTER_SELECT;

    } else if (currentState_ == CHARACTER_SELECT) {
        // this kinda works but input needs to be sorted first
        mainLCD.cls();

        if (!drawn) { 

            // write the new custom characters into memory
            mainLCD.writeCustomCharacter(CharacterSymbols.fighter, 1);
            mainLCD.writeCustomCharacter(CharacterSymbols.mage, 2);
            mainLCD.writeCustomCharacter(CharacterSymbols.ranger, 3);
            mainLCD.writeCustomCharacter(CharacterSymbols.swordman, 4);
            mainLCD.writeCustomCharacter(CharacterSymbols.tank, 5);

            // write selector into memory
            mainLCD.writeCustomCharacter(CharacterSymbols.symbolSelector, 6);

            // dtaw the character icons on screen 
            mainLCD.locate(6, 1);
            mainLCD.printf("%c %c %c %c %c", 0, 1, 2, 3, 4);

            // set drawn = true so these dont have to be re-rendered
            drawn = true;
        }   

        mainLCD.locate(10, 0);
        mainLCD.printf("%c", 5);


    } else if (currentState_ == MENU) {
        
    } else if (currentState_ == FIGHT) {

        for ( int i = 0; i < maxEntities; i++ ) {
        
            if (iconComponent[i].symbol[0] != 0) {
                
                mainLCD.locate(positionComponent[i].x, positionComponent[i].y);
                mainLCD.writeCustomCharacter(iconComponent[i].symbol, i);
                mainLCD.printf("%c", 0);
            }
        }
    }
}

void movementSystem() {

    for (int i = 0; i < maxEntities; ++i) {

        //if (!hasPosition[i]) continue;
        
        if (positionComponent[i].x == 15) positionComponent[i].x = 0;
        if (positionComponent[i].y == 2) positionComponent[i].y = 0;

        positionComponent[i].x += velocityComponent[i].changeX;
        positionComponent[i].y += velocityComponent[i].changeY;

    }
}

// most likely going to run in a separate thread
void inputSystem(GAMESTATE currentState_) {
    
    // Joystick input
    if (mainJoystick.reportPosition() == NULL) return;
        
    // by calculating the magnitude of the joysticks position then checking one of the coordinates we can identify what direction it is pointing
    if ( sqrt(mainJoystick.reportPosition()[0]*mainJoystick.reportPosition()[1]) > 0.75 ) {

    } //else if () {
        //
    //}


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
    

    if (currentState_ == LOADING_SCREEN) {
    
    
    }

}


void systemManager() { 
    // to be completed

    if (currentGameState == LOADING_SCREEN) {

    }
}



int main() {

    //Initiialize Joystick
    initializeEntities(0, 0,0, 1, 0, playerA);
    currentGameState = LOADING_SCREEN;
    currentDirection = NEUTRAL;
    REFRESH_RATE = 1000;
    

    while (true) {
        // uncomment this to get started
        //renderSystemMain(currentGameState);
        
        // just debugging right now
        printf("\n result ");
        for ( float e : mainJoystick.reportPosition() ) {
            printf("%2.4f ", e);
        };

        thread_sleep_for(1000);


        //movementSystem();
        //thread_sleep_for(REFRESH_RATE);
        
    }

}
