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
// DigitalIn selectorButton(D10);
// DigitalIn menuButton(D11);

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

    //Characters  need actualt desgin
    char Charfighter[8] {  0x00, 0x00, 0x02, 0x1E, 0x1E, 0x02, 0x00, 0x00};

    // mana and health
    char manaSymbol[8] { 0x00, 0x00, 0x0E, 0x04, 0x0E, 0x0E, 0x0E, 0x00 };
    char healthSymbol[8] { 0x00, 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00 };
    char halfhealthSymbol[8] { 0x00, 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00 };
    
    
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
bool selectionConfirmed = false;
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


//struct FightAttributes { 
//    int16_t health;
//    int16_t mana;
//};
//FightAttributes fightAttrComponent[maxEntities];
//bool hasFightAttributes[maxEntities];


class CharacterBase
{
protected:
    int Health_;
    int Movement_;
    int Damage_;
    int Position_[2];
    char Type_ = ' ';

public:
    CharacterBase(int Health, int Movement, int Damage, int Position[2], char Type)
    {
        Health_ = Health;
        Movement_ = Movement;
        Damage_ = Damage;
        memcpy(Position_, Position, sizeof(Position_));
        Type_ = Type;
    }

    void UpgradeHp()
        {Health_++;}
        
    void UpgradeM()
        {Movement_++;}

    void UpgradeDamage()
        {Damage_++;}
        
    void TakeDamage(int damge)
    {
        Health_ = Health_ - damge;
    }
        
    int gethp()
        {return Health_;}

    int getMovement()
        {return Movement_;}

    int getDamage()
        {return Damage_;}

    char gettype()
        {return Type_;}

    int getX() { return Position_[0]; }
    int getY() { return Position_[1]; }

    void BasicAttack(){}

    void MagicAttack(){}

    void Block(){}


    void Move(DIRECTION myDirection_) {

        switch(myDirection_) {

            case UP:
            Position_[1] -= 1;
            break;

            case DOWN:
            Position_[1] += 1;
            break;

            case LEFT:
            Position_[0] -= 1;
            break;

            case RIGHT:
            Position_[0] += 1;
            break;

            case NEUTRAL:
            break;

            case NULLPOSITION:
            break;

        }
        
    }

};

class Fighter : public CharacterBase
{
public:
    Fighter(int Health, int Movement, int Damage, int Position[2], char Type)
        : CharacterBase(Health, Movement, Damage, Position,Type)
    { //Stat can be changed just for testing balancing can be done later
        Health_ = Health;
        Movement_ = Movement;
        Damage_ = Damage;
        memcpy(Position_, Position, sizeof(Position_));
        Type_ = Type;
    }
};

class Berserker : public CharacterBase
{
public:
    Berserker(int Health, int Movement, int Damage, int Position[2], char Type)
        : CharacterBase(Health, Movement, Damage, Position,Type)
    {
        Health_ = Health;
        Movement_ = Movement;
        Damage_ = Damage;
        memcpy(Position_, Position, sizeof(Position_));
        Type_ = Type;
    }
};

class Mage : public CharacterBase
{
public:
    Mage(int Health, int Movement, int Damage, int Position[2], char Type)
        : CharacterBase(Health, Movement, Damage, Position,Type)
    {
        Health_ = Health;
        Movement_ = Movement;
        Damage_ = Damage;
        memcpy(Position_, Position, sizeof(Position_));
        Type_ = Type;
    }
};

class Ranger : public CharacterBase
{
public:
    Ranger(int Health, int Movement, int Damage, int Position[2], char Type)
        : CharacterBase(Health, Movement, Damage, Position,Type)
    {
       Health_ = Health;
        Movement_ = Movement;
        Damage_ = Damage;
        memcpy(Position_, Position, sizeof(Position_));
        Type_ = Type;
    }
};

class Tank : public CharacterBase
{
public:
    Tank(int Health, int Movement, int Damage, int Position[2], char Type)
        : CharacterBase(Health, Movement, Damage, Position,Type)
    {
        Health_ = Health;
        Movement_ = Movement;
        Damage_ = Damage;
        memcpy(Position_, Position, sizeof(Position_));
        Type_ = Type;
    }
};

class EnemyBase : public CharacterBase
{
public:

    enum AIMODE {
        MOVE_TO_PLAYER,
        ATTACK,
        FLEE
    };

    AIMODE currentMode = MOVE_TO_PLAYER;

    EnemyBase(int Health, int Movement, int Damage, int Position[2], char Type)
        : CharacterBase(Health, Movement, Damage, Position, Type)
    {
        Health_   = Health;
        Movement_ = Movement;
        Damage_   = Damage;
        memcpy(Position_, Position, sizeof(Position_));
        Type_     = Type;
    }

    void AImove()
    {
        switch (currentMode)
        {
            case MOVE_TO_PLAYER: moveMode();   break;
            case ATTACK:         attackMode(); break;
            case FLEE:           fleeMode();   break;
        }
    }

private:

    // shared Dijkstra — finds the first step toward any target index
    // returns NULLPOSITION if no path exists or enemy is already there
    DIRECTION findFirstStep(int targetIdx)
    {
        const int COLS  = 32;
        const int ROWS  = 2;
        const int TOTAL = COLS * ROWS;

        auto idx = [&](int x, int y) -> int {
            return y * COLS + x;
        };

        int startIdx = idx(Position_[0], Position_[1]);

        if (startIdx == targetIdx) return NULLPOSITION;

        int  dist[TOTAL];
        int  prev[TOTAL];
        bool visited[TOTAL];

        for (int i = 0; i < TOTAL; i++) {
            dist[i]    = INT_MAX;
            prev[i]    = -1;
            visited[i] = false;
        }
        dist[startIdx] = 0;

        for (int iter = 0; iter < TOTAL; iter++) {

            int u = -1;
            for (int i = 0; i < TOTAL; i++) {
                if (!visited[i] && dist[i] != INT_MAX)
                    if (u == -1 || dist[i] < dist[u]) u = i;
            }

            if (u == -1 || u == targetIdx) break;
            visited[u] = true;

            int ux = u % COLS;
            int uy = u / COLS;

            const int dx[4] = {  0, 1,  0, -1 };
            const int dy[4] = { -1, 0,  1,  0 };

            for (int d = 0; d < 4; d++) {
                int nx = ux + dx[d];
                int ny = uy + dy[d];

                if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) continue;

                int v       = idx(nx, ny);
                int newDist = dist[u] + 1;

                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                }
            }
        }

        int cur = targetIdx;
        while (prev[cur] != -1 && prev[cur] != startIdx) {
            cur = prev[cur];
        }

        if (cur == targetIdx && prev[cur] == -1) return NULLPOSITION;

        int cx = cur % COLS, cy = cur / COLS;
        int sx = Position_[0], sy = Position_[1];

        if      (cx > sx) return RIGHT;
        else if (cx < sx) return LEFT;
        else if (cy > sy) return DOWN;
        else              return UP;
    }

    // move toward the player but stop 1 square away
    void moveMode()
    {
        int px = playerCharacter->getX();
        int py = playerCharacter->getY();

        // check manhattan distance to player
        int dist = abs(Position_[0] - px) + abs(Position_[1] - py);

        // already 1 square away — don't move
        if (dist <= 1) return;

        // pick a target 1 square to the left of the player
        // so the enemy closes in but stops adjacent
        int targetX = px - 1;
        int targetY = py;

        // clamp target so it stays inside the 32x2 grid
        if (targetX < 0)  targetX = 0;
        if (targetX > 31) targetX = 31;
        if (targetY < 0)  targetY = 0;
        if (targetY > 1)  targetY = 1;

        const int COLS = 32;
        int targetIdx  = targetY * COLS + targetX;

        DIRECTION step = findFirstStep(targetIdx);
        if (step != NULLPOSITION) Move(step);
    }

    // stay exactly 1 square to the left of the player and hold position
    void attackMode()
    {
        int px = playerCharacter->getX();
        int py = playerCharacter->getY();

        // desired position is 1 square left of the player
        int targetX = px - 1;
        int targetY = py;

        // clamp inside grid
        if (targetX < 0)  targetX = 0;
        if (targetX > 31) targetX = 31;
        if (targetY < 0)  targetY = 0;
        if (targetY > 1)  targetY = 1;

        // already in position — hold still
        if (Position_[0] == targetX && Position_[1] == targetY) return;

        const int COLS = 32;
        int targetIdx  = targetY * COLS + targetX;

        DIRECTION step = findFirstStep(targetIdx);
        if (step != NULLPOSITION) Move(step);
    }

    // flee — move left until hitting the left edge of the map
    void fleeMode()
    {
        if (Position_[0] <= 0) return;  // already at the left edge
        Move(LEFT);
    }
};

CharacterBase* playerCharacter = nullptr;

// Systems

// initialization system

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
        sideLCD.writeCustomCharacter(CharacterSymbols.halfhealthSymbol, 2);
        sideLCD.writeCustomCharacter(CharacterSymbols.manaSymbol, 3);
        
        /*
        for ( int i = 0; i < fightAttrComponent[0].health; i++ ) { 
            sideLCD.locate(0,0);
            sideLCD.printf("%c", i);

        }*/

    } else if (currentState_ == END_SCREEN) {

        sideLCD.cls();
        sideLCD.locate(0, 0);
        sideLCD.printf(" Press any button ");
        
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

        // printf is 0 indexed
        mainLCD.printf("%c%c%c%c%c", 0, 1, 2, 3, 4);    
        mainLCD.locate(2, 2);
        mainLCD.printf("Press any button ");
        mainLCD.locate(5, 3);
        mainLCD.printf("to start..");

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
       
        mainLCD.locate(6, 2);
        mainLCD.printf("%c %c %c %c %c", 0, 1, 2, 3, 4);

    } else if (currentState_ == MENU) {


        
    } else if (currentState_ == MAP) {

         // write the new custom characters into memory *NEED TO Have like rock and shit
        if (playerCharacter.gettype() == 'F')
        {
            mainLCD.writeCustomCharacter(CharacterSymbols.fighter, 1);
        }
        else if (playerCharacter.gettype() == 'B')
        {
            //insert custom character for this class
        }
        else if (playerCharacter.gettype() == 'M')
        {
            //insert custom character for this class
        }
        else if (playerCharacter.gettype() == 'R')
        {
            //insert custom character for this class
        }
        else if (playerCharacter.gettype() == 'T')
        {
            //insert custom character for this class
        }

    } else if (currentState_ == FIGHT) {

        for ( int i = 0; i < maxEntities; i++ ) {
        
            if (iconComponent[i].symbol[0] != 0) {
                
                mainLCD.locate(positionComponent[i].x, positionComponent[i].y);
                mainLCD.writeCustomCharacter(iconComponent[i].symbol, i);
                mainLCD.printf("%c", 0);
            }

        if (playerCharacter.gettype() == 'F')
        {
            mainLCD.writeCustomCharacter(CharacterSymbols.fighter, 1);
        }
        else if (playerCharacter.gettype() == 'B')
        {
            //insert custom character for this class
        }
        else if (playerCharacter.gettype() == 'M')
        {
            //insert custom character for this class
        }
        else if (playerCharacter.gettype() == 'R')
        {
            //insert custom character for this class
        }
        else if (playerCharacter.gettype() == 'T')
        {
            //insert custom character for this class
        }

        }

    } else if (currentState_ == END_SCREEN) {
        mainLCD.cls();
       
        mainLCD.locate(0, 1);
        mainLCD.printf("   GAME OVER   ");
3
    }
}


// most likely going to run in a separate thread
DIRECTION inputSystem() {
    
    // Joystick input
    // by calculating the magnitude of the joysticks position then checking one of the coordinates we can identify what direction it is pointing
   
    int x_fixed { (int)(mainJoystick.reportPosition()[0] * 10) };
    int y_fixed { (int)(mainJoystick.reportPosition()[1] * 10) };
    
    if (x_fixed == 7 && y_fixed == 7) 
        return NEUTRAL;
    else if (x_fixed >= 7) 
    { 
        if (y_fixed >=7) 
            return DOWN; 
        else if (y_fixed < 7) 
            return RIGHT;
    } 
    else if (x_fixed < 7) 
    {
        if (y_fixed >= 7)
            return LEFT;
        else if (y_fixed < 7) 
            return UP;
    }

    return NULLPOSITION;

}

// movementSystem for entities
void movementSystem(GAMESTATE &currentGameState_) {
    currentDirection = inputSystem();

     if (currentGameState_ == CHARACTER_SELECT) {
            
        if (currentDirection == RIGHT) {

            if ( positionComponent[0].x <= 14) positionComponent[0].x  += 2;
           
        } else if (currentDirection == LEFT) {

            if (positionComponent[0].x >= 6 ) positionComponent[0].x  -= 2;
             
        }
    }   else if (currentGameState_ == MAP) {

        playerCharacter -> Move(currentDirection);
    } 
}

void stateManager(GAMESTATE &currentGameState_) {
    // to be completed
    // intended to disable/enable different systems as the game progressed
    // logic for state transitions goes here

    int defaultPos[2] {0, 1};
    currentDirection = inputSystem();

    if (currentGameState_ == LOADING_SCREEN ) {

        if (currentDirection != NEUTRAL) {
            currentGameState_ = CHARACTER_SELECT;
        }

    } else if (currentGameState_ == CHARACTER_SELECT) {

        if ((mainJoystick.reportPush()) && (playerCharacter == nullptr)) {
            
            switch (positionComponent[0].x)//need to get the logic for the position selection and when it presses the button it chooses it  
            {
                case 5: playerCharacter = new Fighter(5, 3, 2, defaultPos, 'F');   break;
                case 7: playerCharacter = new Berserker(5, 3, 2, defaultPos, 'B'); break;
                case 9: playerCharacter = new Mage(5, 3, 2, defaultPos, 'M');      break;
                case 11: playerCharacter = new Ranger(5, 3, 2, defaultPos, 'R');   break;
                case 13: playerCharacter = new Tank(5, 3, 2, defaultPos, 'T');     break;
            }
            selectionConfirmed = true;
        }

    } else if (currentGameState_ == FIGHT) {

    } else if (currentGameState_ == END_SCREEN) {

        if (currentDirection != NEUTRAL) {
            currentGameState_ = LOADING_SCREEN;
        }
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
        inputSystem();
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
        stateManager(currentGameState);

        thread_sleep_for(REFRESH_RATE);        
    }

}
