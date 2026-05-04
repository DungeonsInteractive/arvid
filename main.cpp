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
TextLCD sideLCD(D11, D10, D4, D5, D6, D7, TextLCD::LCD16x2);
TextLCD mainLCD(D9, D8, D0, D1, D2, D3, TextLCD::LCD20x4);

// Joystick for controlling directional movement
Joystick mainJoystick(A0, A1, PC_3);

// Threads

Thread sideLCDWorkerThread;
Thread joystickWorkerThread;

// Buttons for user input
//DigitalIn selectorButton(D10);
//DigitalIn menuButton(D11);

int REFRESH_RATE;
bool drawn = false;
bool drawnB = false;
int start = 0;

struct  {

    // game symbols

    char symbolA[8] { 0x0E, 0x1F, 0x1B, 0x1B, 0x1F, 0x1B, 0x1B, 0x1B };
    char symbolR[8] { 0x1C, 0x1E, 0x1B, 0x19, 0x1F, 0x1B, 0x1B, 0x1B };
    char symbolV[8] { 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x0E };
    char symbolI[8] { 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F };
    char symbolD[8] { 0x1E, 0x1F, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x1E };

    // selector icon
    char symbolSelector[8] = { 0x00, 0x00, 0x00, 0x1F, 0x0E, 0x04, 0x00, 0x00 };

    // class icons
    char fighter[8] { 0x00, 0x00, 0x1F, 0x1F, 0x15, 0x1B, 0x11, 0x00 };
    char ranger[8] { 0x10, 0x08, 0x04, 0x06, 0x06, 0x04, 0x08, 0x10 };
    char mage[8] { 0x00, 0x04, 0x0E, 0x0E, 0x1F, 0x0E, 0x0A, 0x0E };
    char swordman[8] { 0x00, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x04, 0x00 };
    char tank[8] { 0x00, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00 };

    // animation icons
    char swordoneA[8] { 0x00, 0x00, 0x08, 0x0F, 0x0F, 0x08, 0x00, 0x00 };
    char swordoneB[8] { 0x0C, 0x0C, 0x0F, 0x1F, 0x1F, 0x0F, 0x0C, 0x0C };
    char swordoneC[8] { 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00 };
    char swordoneD[8] { 0x00, 0x00, 0x1C, 0x1F, 0x1F, 0x1C, 0x00, 0x00 };

    char swordtwoA[8] {  0x00, 0x00, 0x07,0x1F,0x1F,0x07, 0x00, 0x00};
    char swordtwoB[8] {  0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00};
    char swordtwoC[8] {  0x06, 0x06, 0x1E, 0x1F, 0x1F, 0x1E, 0x06, 0x06};
    char swordtwoD[8] {  0x00, 0x00, 0x02, 0x1E, 0x1E, 0x02, 0x00, 0x00};


    // mana and health
    char manaSymbol[8] { 0x00, 0x00, 0x0E, 0x04, 0x0E, 0x0E, 0x0E, 0x00 };
    char healthSymbol[8] { 0x00, 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00 };
    
    
} CharacterSymbols;


struct {

    // to be filled later 
} GameMaps;

enum DIRECTION {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NEUTRAL,
    NULLPOSITION

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
DIRECTION currentDirection;

bool SELECT[maxEntities];

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
    char symbol[8] {};
};
Icons iconComponent[maxEntities];
bool hasIcon[maxEntities];


struct FightAttributes { 
    int16_t health;
    int16_t mana;
};
FightAttributes fightAttrComponent[maxEntities];
bool hasFightAttributes[maxEntities];


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

char playerA[] {0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F};

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


// render systems for main and side LCDs
void renderSystemSide(GAMESTATE currentState_) {


    if (currentState_ == LOADING_SCREEN) { 
        sideLCD.cls();

        if (!drawnB) {
            
            // sword one
            sideLCD.writeCustomCharacter(CharacterSymbols.swordoneA, 1); // bottom
            sideLCD.writeCustomCharacter(CharacterSymbols.swordoneB, 2);
            sideLCD.writeCustomCharacter(CharacterSymbols.swordoneC, 3);
            sideLCD.writeCustomCharacter(CharacterSymbols.swordoneD, 4); // tip of sword

            // sword two
            sideLCD.writeCustomCharacter(CharacterSymbols.swordtwoA, 5); // tip of sword 
            sideLCD.writeCustomCharacter(CharacterSymbols.swordtwoB, 6); 
            sideLCD.writeCustomCharacter(CharacterSymbols.swordtwoC, 7); 
            sideLCD.writeCustomCharacter(CharacterSymbols.swordtwoD, 8); // bottom

            drawnB = true;
        }   

        sideLCD.locate(start, 0);

        
        if (start == 13) {
            sideLCD.printf("%c%c%c", 0, 1, 2 );
            sideLCD.locate(0, 0);
            sideLCD.printf("%c", 3 );

            sideLCD.locate(0, 1);
            sideLCD.printf("%c%c%c", 5, 6, 7 );
            sideLCD.locate(15, 1);
            sideLCD.printf("%c", 4);

        } else if (start == 14) {
            sideLCD.printf("%c%c", 0, 1 );    
            sideLCD.locate(0, 0);
            sideLCD.printf("%c%c", 2, 3 );

            sideLCD.locate(0, 1);
            sideLCD.printf("%c%c", 6, 7 );
            sideLCD.locate(14, 1);
            sideLCD.printf("%c%c", 4, 5);
          
        } else if (start == 15) {
            sideLCD.printf("%c", 0 );
            sideLCD.locate(0, 0);
            sideLCD.printf("%c%c%c", 1, 2, 3 );

            sideLCD.locate(13, 1);
            sideLCD.printf("%c%c%c", 4, 5, 6 );
            sideLCD.locate(0, 1);
            sideLCD.printf("%c", 7 );

            start = -1;    

        } else {
            sideLCD.printf("%c%c%c%c", 0, 1, 2, 3);
            sideLCD.locate(12 -start, 1);
            sideLCD.printf("%c%c%c%c", 4, 5, 6, 7);
        }
            
        start += 1;
        
    } else if (currentState_ == CHARACTER_SELECT) {
            
            sideLCD.cls();
            
            sideLCD.locate(0, 0);
            sideLCD.printf("%c%c%c%c  %c%c%c%c  %c%c%c%c", 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3);
            sideLCD.locate(1, 1);
            sideLCD.printf("Choose a class");
    } else if (currentState_ == FIGHT) {


        sideLCD.cls();
        sideLCD.writeCustomCharacter(CharacterSymbols.healthSymbol, 1);
        sideLCD.writeCustomCharacter(CharacterSymbols.halfHealthSymbol, 2);
        sideLCD.writeCustomCharacter(CharacterSymbols.manaSymbol, 3);

        for ( int i = 0; i < fightAttrComponent[0].health; i++ ) { 
            sideLCD.locate(0,0);
            sideLCD.printf("%c", i);

        }

    }
}

void renderSystemMain(GAMESTATE &currentState_) {

    // loading screen works
    if (currentState_ == LOADING_SCREEN) {
        mainLCD.cls();

        mainLCD.locate(5, 0);
        mainLCD.printf("Welcome to");
        mainLCD.locate(8, 1);
        
        if (!drawn) {
                
            // remember writeCustomCharacter is not 0 indexed its 1 indexed
            mainLCD.writeCustomCharacter(CharacterSymbols.symbolA, 1);
            mainLCD.writeCustomCharacter(CharacterSymbols.symbolR, 2);
            mainLCD.writeCustomCharacter(CharacterSymbols.symbolV, 3);
            mainLCD.writeCustomCharacter(CharacterSymbols.symbolI, 4);
            mainLCD.writeCustomCharacter(CharacterSymbols.symbolD, 5);
            
            drawn = true;
        }
        // printf is annoyingly 0 indexed
        mainLCD.printf("%c%c%c%c%c", 0, 1, 2, 3, 4);    

        mainLCD.locate(2, 2);
        mainLCD.printf("Press any button ");
        mainLCD.locate(5, 3);
        mainLCD.printf("to start..");


        // place holder for now - should wait for button press to start


        //currentState_ = CHARACTER_SELECT;

    } else if (currentState_ == CHARACTER_SELECT) {
        // this kinda works but input needs to be sorted first
        mainLCD.cls();

        if (drawn) { 

            // write the new custom characters into memory
            mainLCD.writeCustomCharacter(CharacterSymbols.fighter, 1);
            mainLCD.writeCustomCharacter(CharacterSymbols.mage, 2);
            mainLCD.writeCustomCharacter(CharacterSymbols.ranger, 3);
            mainLCD.writeCustomCharacter(CharacterSymbols.swordman, 4);
            mainLCD.writeCustomCharacter(CharacterSymbols.tank, 5);

            // write selector into memory
            mainLCD.writeCustomCharacter(CharacterSymbols.symbolSelector, 6);
            

            // set drawn = true so these dont have to be re-rendered
            drawn = false;
        }
        
        mainLCD.locate(positionComponent[0].x, 1);
        mainLCD.printf("%c", 5);

        
        // draw the character icons on screen 
        //mainLCD.locate(0, 0);
        //mainLCD.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128);
        
        //mainLCD.locate(0, 1);
        //mainLCD.printf("%c",128);
        //mainLCD.locate(19, 1);
        //mainLCD.printf("%c",128);

        //mainLCD.locate(0, 2);
        //mainLCD.printf("%c     %c %c %c %c %c    %c", 128, 0, 1, 2, 3, 4, 128);
        
        mainLCD.locate(6, 2);
        mainLCD.printf("%c %c %c %c %c", 0, 1, 2, 3, 4);


        //mainLCD.locate(0, 3);
        //mainLCD.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128);

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



// most likely going to run in a separate thread
DIRECTION inputSystem(GAMESTATE currentState_) {
    
    // Joystick input
    
    // by calculating the magnitude of the joysticks position then checking one of the coordinates we can identify what direction it is pointing
   
    int x_fixed { (int)(mainJoystick.reportPosition()[0] * 10) };
    int y_fixed { (int)(mainJoystick.reportPosition()[1] * 10) };
    
    if (x_fixed == 7 && y_fixed == 7) return NEUTRAL; else if (x_fixed >= 7) { 
        if (y_fixed >=7) return DOWN; else if (y_fixed < 7) return RIGHT;
    } else if (x_fixed < 7) {
        if (y_fixed >= 7) return LEFT; else if (y_fixed < 7) return UP;
    }

    return NULLPOSITION;

}


// movementSystem for entities

void movementSystem(GAMESTATE &currentGameState_) {

    
    currentDirection = inputSystem(currentGameState_);
    if (currentGameState_ == LOADING_SCREEN ) {

        if (currentDirection != NEUTRAL) {
            currentGameState_ = CHARACTER_SELECT;
        }
    } else if (currentGameState_ == CHARACTER_SELECT) {
            
        if (currentDirection == RIGHT) {

            if ( positionComponent[0].x == 15) { } else positionComponent[0].x  += 2;    
        
        } else if (currentDirection == LEFT) {

            if (positionComponent[0].x == 0) { } else positionComponent[0].x  -= 2;     
             
        }

    }
}

void systemManager() { 
    // to be completed

    if (currentGameState == LOADING_SCREEN) {
         
    }
}


void LCDWorkerThreadFn() {

    REFRESH_RATE = 1000;
    while (true) {
        renderSystemSide(currentGameState);
        thread_sleep_for(REFRESH_RATE);
    }
}

void joystickWorkerThreadFn() {

    while (true) {
        mainJoystick.reportPush();

    }

}

int main() {

    //Initiialize Joystick
    initializeEntities(0, 10, 0, 1, 0, CharacterSymbols.symbolSelector);
    currentGameState = LOADING_SCREEN;
    currentDirection = NEUTRAL;
    REFRESH_RATE = 250;
    
    sideLCDWorkerThread.start(LCDWorkerThreadFn);
    joystickWorkerThread.start(joystickWorkerThreadFn);

    while (true) {


        renderSystemMain(currentGameState);
        movementSystem(currentGameState);
        thread_sleep_for(REFRESH_RATE);        
        
    }

}

