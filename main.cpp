// Arvid pre-release

#include "Joystick.h"
#include "TextLCD_CC.h"
#include "mbed.h"
#include <vector>

using namespace std;
using Entity = uint8_t;


/* DEVICE INITIALISATIONS */
// Initialise main and side LCDs using morpho and digital pins
TextLCD secondaryLCD(D10, D11, D4, D5, D6, D7, TextLCD::LCD16x2), primaryLCD(D9, D8, D0, D1, D2, D3, TextLCD::LCD20x4);

// Joystick for controlling directional movement
Joystick mainJoystick(A0, A1, PC_3);

// Button Inputs
DigitalIn SW1(PA_0, PullUp), menuButton(PA_1, PullUp), SW3(PB_12, PullUp), SW4(PB_1, PullUp),  SW5(PB_11, PullUp), SW6(PA_4, PullUp);

// Threads
Thread secondaryLCDThread;


/* GAME MEDIA */
struct  {
    // game symbols
    char symbolA[8] { 0x0E, 0x1F, 0x1B, 0x1B, 0x1F, 0x1B, 0x1B, 0x1B };
    char symbolR[8] { 0x1C, 0x1E, 0x1B, 0x19, 0x1F, 0x1B, 0x1B, 0x1B };
    char symbolV[8] { 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x0E };
    char symbolI[8] { 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F };
    char symbolD[8] { 0x1E, 0x1F, 0x1B, 0x1B, 0x1B, 0x1B, 0x1F, 0x1E };

    // selector icon
    char symbolSelector[8] = { 0x00, 0x00, 0x00, 0x1F, 0x0E, 0x04, 0x00, 0x00 };

    // character select screen border
    char charSelectBorder[8] { 0x0A, 0x15, 0x0A, 0x0D, 0x16, 0x0A, 0x15, 0x0A };

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
    char halfhealthSymbol[8] { 0x00, 0x00, 0x08, 0x1C, 0x1C, 0x0C, 0x04, 0x00 };
    
    // Map Icons
    char playerIcon[8] { 0x00, 0x00, 0x04, 0x0E, 0x04, 0x04, 0x0A, 0x00 };
    char fightPortal[8] { 0x00, 0x00, 0x0E, 0x02, 0x08, 0x0E, 0x00, 0x00 };
    char bossFightIcon[8] { 0x00, 0x04, 0x0A, 0x11, 0x0A, 0x04, 0x11, 0x0E };
    char restrictedArea[8] { 0x1D, 0x1A, 0x16, 0x0D, 0x16, 0x0D, 0x0B, 0x17 };
    char treasureBox[8] { 0x00, 0x00, 0x0E, 0x00, 0x04, 0x0E, 0x00, 0x00 };
    char enemyIcon[8] {0x00, 0x0E, 0x0A, 0x11, 0x04, 0x0A, 0x00, 0x00 };
    char bossIcon[8] { 0x04, 0x0E, 0x1B, 0x11, 0x04, 0x0A, 0x0A, 0x00 };

} CharacterSymbols;

struct {
    // 20 arays of 4
    // each group of 4 defines a vertical column from top of lcd to bottom 

    // 0 - empty space, 1 - restricted area, 2 - treasure, 3 - enemy, 4 - final boss 5 - player

    // testmap used during development
    char testmap[20][4] {
        {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 0, 0, 1}, {2, 0, 0, 1},
        {2, 0, 0, 1}, {0, 0, 0, 1}, {0, 1, 1, 1}, {3, 0, 0, 1}, {1, 0, 0, 0},
        {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 1, 3, 0}, {1, 2, 0, 1}, {1, 0, 0, 1},
        {1, 2, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 4, 0},
    };

    // defaultPositions = 1 - {0,0}, 2 - {2,1}, 3 - {3,1}, 4 - {5,1}

    char mapOne[20][4] {
        {0, 0, 0, 1}, {0, 0, 3, 1}, {1, 0, 0, 1}, {1, 0, 0, 0}, {2, 1, 0, 0},
        {0, 0, 3, 2}, {1, 1, 2, 0}, {0, 2, 0, 0}, {3, 0, 2, 0}, {0, 0, 0, 1},
        {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 3}, {1, 1, 0, 2}, {1, 1, 0, 1},
        {0, 0, 0, 2}, {1, 0, 0, 1}, {1, 0, 0, 1}, {0, 0, 1, 1}, {4, 0, 0, 1},
    };

    char mapTwo[20][4] {
        {2, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 1, 1}, {3, 0, 0, 1}, {1, 0, 0, 0},
        {0, 2, 0, 1}, {1, 0, 0, 1}, {1, 3, 0, 1}, {1, 0, 2, 1}, {1, 0, 0, 0},
        {1, 1, 0, 1}, {1, 1, 2, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {2, 0, 0, 1},
        {0, 2, 1, 0}, {0, 0, 0, 0}, {0, 0, 3, 0}, {1, 2, 4, 1}, {1, 0, 0, 1}
        
    };
    
    char mapThree[20][4] {
        {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 2, 0}, {1, 2, 0, 1}, {1, 3, 0, 1},
        {0, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 2, 3, 1}, {0, 0, 0, 0},
        {0, 0, 1, 1}, {0, 1, 3, 0}, {1, 0, 0, 1}, {2, 0, 0, 1}, {3, 0, 0, 1},
        {0, 0, 0, 1}, {0, 2, 0, 1}, {0, 1, 2, 1}, {3, 4, 0, 0}, {2, 2, 0, 0},
    };

    char mapFour[20][4] {
        {1, 0, 0, 1}, {1, 0, 2, 0}, {0, 3, 0, 0}, {1, 0, 1, 1}, {2, 0, 0, 0},
        {0, 0, 3, 0}, {0, 3, 2, 0}, {0, 2, 2, 1}, {1, 0, 0, 1}, {0, 0, 2, 0},
        {0, 0, 0, 1}, {0, 0, 2, 1}, {0, 1, 2, 0}, {3, 2, 0, 0}, {2, 3, 0, 3},
        {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 3}, {1, 0, 0, 2}, {1, 1, 0, 4},
    };

    // fight map (all zeroes)
    char fightMap[20][4] {};

} GameMaps;

// 20*4 vector matrices to store previous and current map state in different gamestates
// these help optimise rendering
vector <vector<int>> currentMap(20, vector<int>(4)), prevMap(20, vector<int>(4)), currentFightMap(20, vector<int>(4)), prevFightMap(20, vector<int>(4));
vector <int> selectorTracker(2);


/* ENUMS */
enum DIRECTION {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NEUTRAL,
    NULLPOSITION
};

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


/* SYSTEM VARIABLES */
int REFRESH_RATE;
bool clearScreen;
int swordLocation;
int chosenMap;
bool bossFight;
bool fightMode;
bool victory;
vector <int> preFightPos = {0 , 0};

GAMESTATE currentGameState;
DIRECTION currentDirection;


// Entities

// Helper Entities
const uint16_t maxHelperEntities = 4;

// Character Entities
// Base class from which player and enemy inherits from
class BaseEntity {
    protected:
        int Health_;
        int Movement_;
        int Damage_;
        int Position_[2];
        char Type_ = ' ';

    public:
        BaseEntity(int Health, int Movement, int Damage, vector<int> Position, char Type)
        {
            Health_ = Health;
            Movement_ = Movement;
            Damage_ = Damage;
            Position_[0] = Position[0];
            Position_[1] = Position[1];
            Type_ = Type;
        }

        void UpgradeHp()
            { if (Health_ < 10) Health_++;}
            
        void UpgradeM()
            { if (Movement_ < 10) Movement_++;}

        void UpgradeDamage()
            { if (Damage_ < 5) Damage_++;}
            
        void TakeDamage(int damage) 
            {Health_ = Health_ - damage;}
            
        int gethp()
            {return Health_;}

        int getMovement()
            {return Movement_;}

        int getDamage()
            {return Damage_;}

        char gettype()
            {return Type_;}

        int getPositionX() 
            {return Position_[0];}

        int getPositionY()
            {return Position_[1];}

        void setPositionX(int x)
            {Position_[0] = x;}

        void setPositionY(int y)
            {Position_[1] = y;}


        // attack functions to be done later
        void BasicAttack() {}

        void MagicAttack() {}

        void Block() {}

        void Move(DIRECTION myDirection_) {
            
            // depending on if fight mode is enabled we move on the fight map or regular map
            if (fightMode) {
                switch(myDirection_) {

                case UP:
                    if ((Position_[1] > 0) && currentFightMap[getPositionX()][getPositionY()-1] != 6) Position_[1] -= 1;
                break;

                case DOWN:
                    if ((Position_[1] < 3) && currentFightMap[getPositionX()][getPositionY()+1] != 6) Position_[1] += 1;
                break;

                case LEFT:
                    if ((Position_[0] > 0) && currentFightMap[getPositionX()-1][getPositionY()] != 6) Position_[0] -= 1;
                break;

                case RIGHT:
                    if ((Position_[0] < 19) && currentFightMap[getPositionX()+1][getPositionY()] != 6) Position_[0] += 1;
                break;

                case NEUTRAL:
                break;

                case NULLPOSITION:
                break;
            }   
            } else {
                switch(myDirection_) {

                case UP:
                    if ((Position_[1] > 0) && currentMap[getPositionX()][getPositionY()-1] != 1) Position_[1] -= 1;
                break;

                case DOWN:
                    if ((Position_[1] < 3) && currentMap[getPositionX()][getPositionY()+1] != 1) Position_[1] += 1;
                break;

                case LEFT:
                    if ((Position_[0] > 0) && currentMap[getPositionX()-1][getPositionY()] != 1) Position_[0] -= 1;
                break;

                case RIGHT:
                    if ((Position_[0] < 19) && currentMap[getPositionX()+1][getPositionY()] != 1) Position_[0] += 1;
                break;

                case NEUTRAL:
                break;

                case NULLPOSITION:
                break;
            }   
            }
            
        }
};


// Player classes for each combat class
class Fighter : public BaseEntity {
    public:
        Fighter(int Health, int Movement, int Damage, vector<int> Position, char Type)
            : BaseEntity(Health, Movement, Damage, Position,Type)
        { //Stat can be changed just for testing balancing can be done later
            Health_ = Health;
            Movement_ = Movement;
            Damage_ = Damage;
            Position_[0] = Position[0];
            Position_[1] = Position[1];
            Type_ = Type;
        }
};

class Berserker : public BaseEntity {
    public:
        Berserker(int Health, int Movement, int Damage, vector<int> Position, char Type)
            : BaseEntity(Health, Movement, Damage, Position,Type)
        {
            Health_ = Health;
            Movement_ = Movement;
            Damage_ = Damage;
            Position_[0] = Position[0];
            Position_[1] = Position[1];
            Type_ = Type;
        }
};

class Mage : public BaseEntity {
    public:
        Mage(int Health, int Movement, int Damage, vector<int> Position, char Type)
            : BaseEntity(Health, Movement, Damage, Position,Type)
        {
            Health_ = Health;
            Movement_ = Movement;
            Damage_ = Damage;
            Position_[0] = Position[0];
            Position_[1] = Position[1];
            Type_ = Type;
        }
};

class Ranger : public BaseEntity {
    public:
        Ranger(int Health, int Movement, int Damage, vector<int> Position, char Type)
            : BaseEntity(Health, Movement, Damage, Position,Type)
        {
        Health_ = Health;
            Movement_ = Movement;
            Damage_ = Damage;
            Position_[0] = Position[0];
            Position_[1] = Position[1];
            Type_ = Type;
        }
};

class Tank : public BaseEntity {
    public:
        Tank(int Health, int Movement, int Damage, vector<int> Position, char Type)
            : BaseEntity(Health, Movement, Damage, Position,Type)
        {
            Health_ = Health;
            Movement_ = Movement;
            Damage_ = Damage;
            Position_[0] = Position[0];
            Position_[1] = Position[1];
            Type_ = Type;
        }
};

BaseEntity* playerCharacter = nullptr;

// Enemy class
class EnemyBase : public BaseEntity {
    public:

        enum AIMODE {
            MOVE_TO_PLAYER,
            ATTACK,
            FLEE
        };

        AIMODE currentMode = MOVE_TO_PLAYER;

        EnemyBase(int Health, int Movement, int Damage, vector<int> Position, char Type)
            : BaseEntity(Health, Movement, Damage, Position, Type)
        {
            Health_   = Health;
            Movement_ = Movement;
            Damage_   = Damage;
            Position_[0] = Position[0];
            Position_[1] = Position[1];
            Type_     = Type;
        }

        void AImove() {
            switch (currentMode)
            {
                case MOVE_TO_PLAYER: moveMode();   break;
                case ATTACK:         attackMode(); break;
                case FLEE:           fleeMode();   break;
            }
        }

        void setCurrentMode(AIMODE currentMode_) {
            currentMode = currentMode_;
        }

    private:

        // shared Dijkstra — finds the first step toward any target index
        // returns NULLPOSITION if no path exists or enemy is already there
        DIRECTION findFirstStep(int targetIdx) {
            const int COLS  = 20;
            const int ROWS  = 4;
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

                    int v = idx(nx, ny);
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

            if (cx > sx) return RIGHT;
            else if (cx < sx) return LEFT;
            else if (cy > sy) return DOWN;
            else return UP;

        }

        // move toward the player but stop 1 square away
        void moveMode() {
            int px = playerCharacter->getPositionX();
            int py = playerCharacter->getPositionY();

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
            if (targetX > 19) targetX = 19;
            if (targetY < 0)  targetY = 0;
            if (targetY > 1)  targetY = 1;

            const int COLS = 19;
            int targetIdx  = targetY * COLS + targetX;

            DIRECTION step = findFirstStep(targetIdx);
            if (step != NULLPOSITION) Move(step);
        }

        // stay exactly 1 square to the left of the player and hold position
        void attackMode() {
            int px = playerCharacter->getPositionX();
            int py = playerCharacter->getPositionY();

            // desired position is 1 square left of the player
            int targetX = px - 1;
            int targetY = py;

            // clamp inside grid
            if (targetX < 0)  targetX = 0;
            if (targetX > 19) targetX = 19;
            if (targetY < 0)  targetY = 0;
            if (targetY > 1)  targetY = 1;

            // already in position — hold still
            if (Position_[0] == targetX && Position_[1] == targetY) return;

            const int COLS = 19;
            int targetIdx  = targetY * COLS + targetX;

            DIRECTION step = findFirstStep(targetIdx);
            if (step != NULLPOSITION) Move(step);
        }

        // flee — move left until hitting the left edge of the map
        void fleeMode() {
            if (Position_[0] <= 0) return;  // already at the left edge
            Move(LEFT);
        }
};

EnemyBase* enemyCharacter = nullptr;


// Components (basically store the attributes/things an entity has)
// Boolean arrays keep track of what helper entities have what components

struct Position {
  uint16_t x;
  uint16_t y;  
};
Position positionComponent[maxHelperEntities];
bool hasPosition[maxHelperEntities];

struct Velocity {
    uint16_t dX;
    uint16_t dY;
};
Velocity velocityComponent[maxHelperEntities];
bool hasVelocity[maxHelperEntities];

struct Icons { 
    char symbol[8] {};
};
Icons iconComponent[maxHelperEntities];
bool hasIcon[maxHelperEntities];

// helper function to initialise new entities
void createHelperEntity(Entity id, uint16_t positionX_, uint16_t positionY_, uint16_t velocityX_, uint16_t velocityY_,  const char * symbol_) { 
    
    // formerly used to set entitiy attributes
    // now used to set attributes for particular entities -> selector icon
    positionComponent[id].x = positionX_;
    positionComponent[id].y = positionY_;
    
    velocityComponent[id].dX = velocityX_;
    velocityComponent[id].dY = velocityY_;

    // sets entity icon
    memcpy(iconComponent[id].symbol, symbol_, 8);
}


/* SYSTEMS AND THREADS */

// helper function to get Joystick's direction
DIRECTION joystickInputSystem() {
    
    // Joystick input
    // this works by mapping the joystick voltage readings to different directions
    // joystick is rotated by 45 degrees on mount to make this easier
    
    // type casting allows us perform equality checks with integer values
    // this captures cases with very slight differences e.g. comparing (0.7002 and 0.7120 volts)
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

// render systems for main and side LCDs
void secondaryRenderSystem() {

    if (currentGameState == LOADING_SCREEN) { 

        secondaryLCD.cls();

        if (clearScreen) {
            // this section of code writes the custom characters for parts of the swords into memory

            // sword one
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneA, 1); // bottom of sword
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneB, 2);
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneC, 3);
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneD, 4); // tip of sword 2

            // sword two
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoA, 5); // tip of sword 2
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoB, 6); 
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoC, 7); 
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoD, 8); // bottom of sword 2
        }   

        // the following sequence draws the swords in different positions depending on the new location
        // the if statements handle transitions in and out of the screen
        // eaach if statement is structured as top half -> top sword, bottom half -> bottom sword

        secondaryLCD.locate(swordLocation, 0);
    
        if (swordLocation == 13) {
            secondaryLCD.printf("%c%c%c", 0, 1, 2 );
            secondaryLCD.locate(0, 0);
            secondaryLCD.printf("%c", 3 );

            secondaryLCD.locate(0, 1);
            secondaryLCD.printf("%c%c%c", 5, 6, 7 );
            secondaryLCD.locate(15, 1);
            secondaryLCD.printf("%c", 4);

        } else if (swordLocation == 14) {
            secondaryLCD.printf("%c%c", 0, 1 );    
            secondaryLCD.locate(0, 0);
            secondaryLCD.printf("%c%c", 2, 3 );

            secondaryLCD.locate(0, 1);
            secondaryLCD.printf("%c%c", 6, 7 );
            secondaryLCD.locate(14, 1);
            secondaryLCD.printf("%c%c", 4, 5);
          
        } else if (swordLocation == 15) {
            secondaryLCD.printf("%c", 0 );
            secondaryLCD.locate(0, 0);
            secondaryLCD.printf("%c%c%c", 1, 2, 3 );

            secondaryLCD.locate(13, 1);
            secondaryLCD.printf("%c%c%c", 4, 5, 6 );
            secondaryLCD.locate(0, 1);
            secondaryLCD.printf("%c", 7 );

            // this resets the swordLocation so the animation can continue
            // -1 is chosen because the increment would take it to 0 (the appropriate location)
            swordLocation = -1;

        } else {
            // this else statement handles cases where the swords are not broken up
            secondaryLCD.printf("%c%c%c%c", 0, 1, 2, 3);
            secondaryLCD.locate(12 - swordLocation, 1);
            secondaryLCD.printf("%c%c%c%c", 4, 5, 6, 7);
        }
        
        // this increment moves the swords to the next position
        swordLocation += 1;
        
    } else if (currentGameState == CHARACTER_SELECT) {

        // clearScreen var check not used here

        // clears screen after first transiton into CHARACTER_SELECT state
        secondaryLCD.cls();

        // prints swords and text onto the screen
        secondaryLCD.locate(0, 0);
        secondaryLCD.printf("%c%c%c%c  %c%c%c%c  %c%c%c%c", 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3);
        secondaryLCD.locate(1, 1);
        secondaryLCD.printf("Choose a class");

    } else if (currentGameState == MAP) {

        if (clearScreen) {
            // writes the health symbol into secondary LCD's memory after first transition into state
            secondaryLCD.writeCustomCharacter(CharacterSymbols.healthSymbol, 1);
            secondaryLCD.writeCustomCharacter(CharacterSymbols.halfhealthSymbol, 2);
        }

        // keeping cls function outside of clearScreen causes the flashing text animation
        secondaryLCD.cls();
        secondaryLCD.locate(0, 0);
        secondaryLCD.printf("Reach the BOSS");

        // displays player health on screen
        secondaryLCD.locate(0, 1);
        secondaryLCD.printf("H:");

        // prints full hearts for even health
        for (int i = 0; i < (playerCharacter->gethp()/2); i++) {
            secondaryLCD.locate(i+2, 1);
            secondaryLCD.printf("%c", 0);
        }

        // prints half heart for odd health
        if (playerCharacter->gethp()%2 == 1) {
            secondaryLCD.locate(playerCharacter->gethp()/2 + 2, 1);
            secondaryLCD.printf("%c", 1);
        }

    } else if (currentGameState == FIGHT) {

        if (clearScreen) {  
            // writes custom characters for health back into memory
            secondaryLCD.writeCustomCharacter(CharacterSymbols.healthSymbol, 1);
            secondaryLCD.writeCustomCharacter(CharacterSymbols.halfhealthSymbol, 2);
            secondaryLCD.writeCustomCharacter(CharacterSymbols.manaSymbol, 3);
        }

        secondaryLCD.cls();
        
        // player health
        secondaryLCD.locate(0,0);
        secondaryLCD.printf("CH ");
       
        // prints full hearts for even health
        for (int i = 0; i < (playerCharacter->gethp()/2); i++) {
            secondaryLCD.locate(i+2, 0);
            secondaryLCD.printf("%c", 0);
        }
        // prints half heart for odd health
        if (playerCharacter->gethp()%2 == 1) {
            secondaryLCD.locate(playerCharacter->gethp()/2 + 2, 0);
            secondaryLCD.printf("%c", 1);
        }

        // enemy health
        secondaryLCD.locate(0,1);
        secondaryLCD.printf("EH ");
       
        // prints full hearts for even health
        for (int i = 0; i < (enemyCharacter->gethp()/2); i++) {
            secondaryLCD.locate(i+2, 1);
            secondaryLCD.printf("%c", 0);
        }
        // prints half heart for odd health
        if (enemyCharacter->gethp()%2 == 1) {
            secondaryLCD.locate(enemyCharacter->gethp()/2 + 2, 1);
            secondaryLCD.printf("%c", 1);
        }

    } else if (currentGameState == END_SCREEN) {
        secondaryLCD.cls();

        if (clearScreen) {
            // write swords into LCD memory once again
            // sword one
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneA, 1); // bottom
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneB, 2);
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneC, 3);
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordoneD, 4); // tip of sword

            // sword two
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoA, 5); // tip of sword 
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoB, 6); 
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoC, 7); 
            secondaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoD, 8); // bottom
        }

        secondaryLCD.locate(0, 0);
        secondaryLCD.printf("%c%c%c%c  Play", 0, 1, 2, 3);
        secondaryLCD.locate(5, 1);
        secondaryLCD.printf("Again?  %c%c%c%c", 4, 5, 6, 7);

    }
}

void secondaryLCDThreadFn() {

    while (true) {
        secondaryRenderSystem();
        thread_sleep_for(REFRESH_RATE);
    }
}

void primaryRenderSystem() {

    // loading screen works
    if (currentGameState == LOADING_SCREEN) {
        primaryLCD.cls();

        primaryLCD.locate(5, 0);
        primaryLCD.printf("Welcome to");
        primaryLCD.locate(8, 1);
        
        if (clearScreen) {
                
            // remember writeCustomCharacter is not 0 indexed its 1 indexed
            primaryLCD.writeCustomCharacter(CharacterSymbols.symbolA, 1);
            primaryLCD.writeCustomCharacter(CharacterSymbols.symbolR, 2);
            primaryLCD.writeCustomCharacter(CharacterSymbols.symbolV, 3);
            primaryLCD.writeCustomCharacter(CharacterSymbols.symbolI, 4);
            primaryLCD.writeCustomCharacter(CharacterSymbols.symbolD, 5);
            
            clearScreen = false;
        }

        // printf is 0 indexed
        primaryLCD.printf("%c%c%c%c%c", 0, 1, 2, 3, 4);    
        primaryLCD.locate(2, 2);
        primaryLCD.printf("Press any button ");
        primaryLCD.locate(5, 3);
        primaryLCD.printf("to start..");

    } else if (currentGameState == CHARACTER_SELECT) {

        if (clearScreen) { 

            primaryLCD.cls();

            // write the new custom characters into memory
            primaryLCD.writeCustomCharacter(CharacterSymbols.fighter, 1);
            primaryLCD.writeCustomCharacter(CharacterSymbols.mage, 2);
            primaryLCD.writeCustomCharacter(CharacterSymbols.ranger, 3);
            primaryLCD.writeCustomCharacter(CharacterSymbols.swordman, 4);
            primaryLCD.writeCustomCharacter(CharacterSymbols.tank, 5);
        
            // write selector symbol into memory
            primaryLCD.writeCustomCharacter(CharacterSymbols.symbolSelector, 6);

            // write border symbol into memory
            primaryLCD.writeCustomCharacter(CharacterSymbols.charSelectBorder, 7);

            // print borders onto screen
            primaryLCD.locate(0, 0);
            primaryLCD.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6);
            primaryLCD.locate(0, 1);
            primaryLCD.printf("%c", 6);
            primaryLCD.locate(19, 1);
            primaryLCD.printf("%c", 6);
            primaryLCD.locate(0, 3);
            primaryLCD.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6);

            // prints the class icons unto the screen
            primaryLCD.locate(0, 2);
            primaryLCD.printf("%c     %c %c %c %c %c    %c", 6, 0, 1, 2, 3, 4, 6);

            // set clearScreen tp false so these don't have to be re-rendered
            clearScreen = false;
        }

        // triggers redraw only when selector position has been altered
        if (selectorTracker[0] != selectorTracker[1]) {
            // prints empty space on old position
            primaryLCD.locate(selectorTracker[1], 1);
            primaryLCD.printf(" ");
            
            // prints selector at new position
            primaryLCD.locate(selectorTracker[0], 1);
            primaryLCD.printf("%c", 5);
        }
        
    } else if (currentGameState == MAP) {

        if (clearScreen) {
            
            primaryLCD.cls();

            // write the custom character for chosen class into memory
            if (playerCharacter->gettype() == 'F') {
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);
            } else if (playerCharacter->gettype() == 'B') {
                //insert custom character for this class 
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);

            } else if (playerCharacter->gettype() == 'M') {
                //insert custom character for this class
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);

            } else if (playerCharacter->gettype() == 'R') {
                //insert custom character for this class
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);

            } else if (playerCharacter->gettype() == 'T') {
                //insert custom character for this class
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);

            }

            
            // invalidates the map so redraw gets triggered
            for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) prevMap[x][y] -= 1;

            // write custom characters for other map elements into memory
            primaryLCD.writeCustomCharacter(CharacterSymbols.fightPortal, 2);
            primaryLCD.writeCustomCharacter(CharacterSymbols.bossFightIcon, 3);
            primaryLCD.writeCustomCharacter(CharacterSymbols.treasureBox, 4);
            primaryLCD.writeCustomCharacter(CharacterSymbols.restrictedArea, 5);

            
            clearScreen = false;
        }
   
        for (int y = 0; y < 4; y++) {
            for ( int x = 0; x < 20; x++) {

                if (prevMap[x][y] != currentMap[x][y]) {
                    primaryLCD.locate(x, y);
                    
                    switch(currentMap[x][y]) {

                        case 0:
                        primaryLCD.printf(" ");
                        break;

                        case 1:
                        // prints restricted area
                        primaryLCD.printf("%c", 4);
                        break;

                        case 2:
                        // prints treasure box
                        primaryLCD.printf("%c", 3);
                        break;

                        case 3:
                        // prints fight portal
                        primaryLCD.printf("%c", 1);
                        break;

                        case 4:
                        // prints bossfight portal
                        primaryLCD.printf("%c", 2);
                        break;
           
                        case 5:
                        // prints player icon
                        primaryLCD.printf("%c", 0);
                    }
                }
            }
        }

    } else if (currentGameState == FIGHT) {

        if (clearScreen) {

            primaryLCD.cls();

            // write the custom character for chosen class into memory
            if (playerCharacter->gettype() == 'F') {
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);
            } else if (playerCharacter->gettype() == 'B') {
                // insert custom character for this class 
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);

            } else if (playerCharacter->gettype() == 'M') {
                // insert custom character for this class
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);

            } else if (playerCharacter->gettype() == 'R') {
                // insert custom character for this class
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);

            } else if (playerCharacter->gettype() == 'T') {
                // insert custom character for this class
                primaryLCD.writeCustomCharacter(CharacterSymbols.playerIcon, 1);
            }
            
            if (bossFight) {
                // write the custom character for the boss into memory
                primaryLCD.writeCustomCharacter(CharacterSymbols.bossIcon, 2);
            } else {                    
                // write the custom character for the enemy into memory
                primaryLCD.writeCustomCharacter(CharacterSymbols.enemyIcon, 2);
            }

            clearScreen = false;
        }

        for (int y = 0; y < 4; y++) {
            for ( int x = 0; x < 20; x++) {
                if (prevFightMap[x][y] != currentFightMap[x][y]) {
                    primaryLCD.locate(x, y);
                    switch(currentFightMap[x][y]) {
                        case 0:
                        // prints empty space
                        primaryLCD.printf(" ");
                        break;
                        case 5:
                        // prints player icon
                        primaryLCD.printf("%c", 0);
                        break;
                        case 6:
                        // prints enemy or boss icon
                        primaryLCD.printf("%c", 1);
                        break;         
                    }
                }
            }
        }

    } else if (currentGameState == END_SCREEN) {

        if (victory) {
            
            if (clearScreen) {
                primaryLCD.cls();

                // sword one
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordoneA, 1); // bottom
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordoneB, 2);
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordoneC, 3);
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordoneD, 4); // tip of sword

                // sword two
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoA, 5); // tip of sword 
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoB, 6); 
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoC, 7); 
                primaryLCD.writeCustomCharacter(CharacterSymbols.swordtwoD, 8); // bottom

                clearScreen = false;
            }

            // prints sword border
            primaryLCD.locate(0, 0);
            primaryLCD.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3);
            
            primaryLCD.locate(0, 3);
            primaryLCD.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);


            primaryLCD.locate(1, 1);
            primaryLCD.printf("<--- VICTORY --->");

        } else {
            if (clearScreen) {
                    
                primaryLCD.cls();
                primaryLCD.locate(4, 2);
                primaryLCD.printf("YOU LOSE");
                
                primaryLCD.locate(1, 3);
                primaryLCD.printf("=== START OVER ===");
            }            
        }
    }    
}

// movementSystem manages movement for all entities
void movementSystem() {
    currentDirection = joystickInputSystem();
    
    if (currentGameState == CHARACTER_SELECT) {
        
        if (currentDirection == RIGHT) {
            selectorTracker[1] = positionComponent[0].x;
            // prevents cursor from exceeding rightmost class icon
            if (positionComponent[0].x <= 12) {
                positionComponent[0].x  += 2;
                selectorTracker[0] = positionComponent[0].x;
            }

        } else if (currentDirection == LEFT) {
            selectorTracker[1] = positionComponent[0].x;
            // prevents the cursor from exceeding leftmost class icon
            if (positionComponent[0].x >= 8 ) {
                positionComponent[0].x  -= 2;
                selectorTracker[0] = positionComponent[0].x;
            }     
        }

    } else if (currentGameState == MAP) {

        if (currentDirection != NEUTRAL) {
            for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) prevMap[x][y] = currentMap[x][y];
            currentMap[playerCharacter->getPositionX()][playerCharacter->getPositionY()] = 0;
            playerCharacter -> Move(currentDirection);            
            currentMap[playerCharacter->getPositionX()][playerCharacter->getPositionY()] = 5;
        }

    } else if (currentGameState == FIGHT) {
        
        // player movement
        if (currentDirection != NEUTRAL) {
            for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) prevFightMap[x][y] = currentFightMap[x][y];
            currentFightMap[playerCharacter->getPositionX()][playerCharacter->getPositionY()] = 0;
            playerCharacter -> Move(currentDirection);            
            currentFightMap[playerCharacter->getPositionX()][playerCharacter->getPositionY()] = 5;
        }

        if (enemyCharacter != nullptr) {
            // enemy movement
            currentFightMap[enemyCharacter->getPositionX()][enemyCharacter->getPositionY()] = 0;

            // enemy logic here
            // generates random number from 0 to 3 i.e {0, 1, 2, 3}
            int action = rand() % 4;
            switch (action) {
                case 0:
                enemyCharacter->setCurrentMode(EnemyBase::MOVE_TO_PLAYER);
                enemyCharacter->AImove();
                break;

                case 1:
                enemyCharacter->setCurrentMode(EnemyBase::FLEE);
                enemyCharacter->AImove();
                if (action%2 == 0) enemyCharacter->UpgradeHp(); else enemyCharacter->UpgradeDamage();
                break;

                case 2:
                enemyCharacter->setCurrentMode(EnemyBase::ATTACK);
                enemyCharacter->AImove();
                playerCharacter->TakeDamage(2);
                break;

                case 3:            
                enemyCharacter->setCurrentMode(EnemyBase::MOVE_TO_PLAYER);
                enemyCharacter->AImove();
                break;
            }

            currentFightMap[enemyCharacter->getPositionX()][enemyCharacter->getPositionY()] = 6;
        }
    }

}

// core functionality and logic
void stateManager() {
    // logic for state transitions goes here

    vector <vector<int>> defaultPositions(4, vector<int>(2));
    defaultPositions = {{0,0}, {2,1}, {3,2}, {5,3}};
    vector<int> defaultEnemyPosition {15, 1};
    currentDirection = joystickInputSystem();

    if (currentGameState == LOADING_SCREEN) {

        if ((currentDirection != NEUTRAL) || !SW3 || !SW4 || !SW5 || !SW5 || !SW6) {
            currentGameState = CHARACTER_SELECT;
            clearScreen = true;
        }

    } else if (currentGameState == CHARACTER_SELECT) {

        if ((!SW3) && (playerCharacter == nullptr)) {
            switch (positionComponent[0].x) {
                // Health, Movement, Damage, Position[2], Type
                case 6: 
                playerCharacter = new Fighter(8, 1, 2, defaultPositions[chosenMap], 'F');
                break;
                              
                case 8: 
                playerCharacter = new Mage(5, 3, 2, defaultPositions[chosenMap], 'M');
                break;

                case 10:
                playerCharacter = new Ranger(3, 3, 5, defaultPositions[chosenMap], 'R');
                break;

                case 12: 
                playerCharacter = new Berserker(4, 2, 4, defaultPositions[chosenMap], 'B');
                break;
                
                case 14:
                playerCharacter = new Tank(10, 1, 1, defaultPositions[chosenMap], 'T');
                break;
            }

            // copies the selected gameMap into the currentMap variable
            switch (chosenMap) {
                case 0:
                    for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) currentMap[x][y] = GameMaps.mapOne[x][y];
                break;
                case 1:
                    for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) currentMap[x][y] = GameMaps.mapTwo[x][y];
                break;
                case 2:
                    for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) currentMap[x][y] = GameMaps.mapThree[x][y];
                break;
                case 3:
                    for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) currentMap[x][y] = GameMaps.mapFour[x][y];
                break;
            }

            currentGameState = MAP;
            clearScreen = true;
        }

    } else if (currentGameState == MAP) {

        // transitions to two different fights depending on portal you enter
        switch (prevMap[playerCharacter->getPositionX()][playerCharacter->getPositionY()]) {
            
            case 2:
                playerCharacter->UpgradeDamage();
                playerCharacter->UpgradeHp();
            break;

            case 3:
                fightMode = true;
                bossFight = false;

                // save the players current position
                preFightPos[0] = playerCharacter->getPositionX(); 
                preFightPos[1] = playerCharacter->getPositionY();

                // deletes the portal by setting it to empty space after its triggered
                currentMap[preFightPos[0]][preFightPos[1]] = prevMap[preFightPos[0]][preFightPos[1]] = 0; 

                // set player position for fight
                playerCharacter->setPositionX(2);
                playerCharacter->setPositionY(1);

                // create the enemy
                enemyCharacter = new EnemyBase(6, 1, 2, defaultEnemyPosition, 'G');

                // initialise the fight map
                for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) currentFightMap[x][y] = prevFightMap[x][y] = 0;

                // place enemy and player on the map
                currentFightMap[2][1] = 5;
                currentFightMap[defaultEnemyPosition[0]][defaultEnemyPosition[1]] = 6;

                currentGameState = FIGHT;
                clearScreen = true;
            break;

            case 4:
                fightMode = true;
                bossFight = true;

                // no need to delete portal or save position here
            
                // set player position for fight
                playerCharacter->setPositionX(2);
                playerCharacter->setPositionY(1);

                // create the boss
                enemyCharacter = new EnemyBase(10, 3, 2, defaultEnemyPosition, 'B');

                // initialise the fight map
                for (int y = 0; y < 4; y++) for (int x = 0; x < 20; x++) currentFightMap[x][y] = prevFightMap[x][y] = 0;
                
                // place boss and player on the map
                currentFightMap[2][1] = 5;
                currentFightMap[defaultEnemyPosition[0]][defaultEnemyPosition[1]] = 6;

                currentGameState = FIGHT;
                clearScreen = true;
            break;
        }
    } else if (currentGameState == FIGHT) {
  
        // exit logic for enemy dying
        if ((enemyCharacter != NULL) && (enemyCharacter->gethp() <= 0)) {
            // set fightmode = false to go back to MAP movement mode
            fightMode = false;
            delete enemyCharacter;
            enemyCharacter = nullptr;

            if (bossFight) {

                // transitions to victory screen after you defeat boss
                // reset all system variables so you can play through again
                currentGameState = END_SCREEN;
                delete playerCharacter;
                victory = true;
                playerCharacter = nullptr;
                bossFight = false;
                swordLocation = 0;
                chosenMap = rand()%4;

                // clear screen for next state to be rendered
                clearScreen = true;
                return;
            }            

            // reconciles player position with map
            playerCharacter->setPositionX(preFightPos[0]);
            playerCharacter->setPositionY(preFightPos[1]);

            currentMap[preFightPos[0]][preFightPos[1]] = 5;
            // trigger screen clear and make current game state map
        
            currentGameState = MAP;
            clearScreen = true;
            return;
        }

        // exit logic for player dying
        if ((playerCharacter != NULL) && (playerCharacter->gethp() <= 0)) {
            // set fightmode = false to go back to MAP movement mode
            fightMode = false;
            victory = false;
            delete enemyCharacter;
            enemyCharacter = nullptr;

            delete playerCharacter;
            playerCharacter = nullptr;

            // trigger screen clear and make current game state map
            currentGameState = END_SCREEN;
            clearScreen = true;
            return;
        }
        // logic for each button during fight
        // basic attack
        if (!SW3) {
            enemyCharacter->TakeDamage(1);
        }
        // magic attack
        if (!SW4) {
            enemyCharacter->TakeDamage(2);
        }
        // defense
        if (!SW5) {
            playerCharacter->Block();
        }
        // heal
        if (!SW6) {
            playerCharacter->UpgradeHp();
        } 
    } else if (currentGameState == END_SCREEN) {

        // when SW1 is pressed trigger game restart
        if (!SW1) {
            currentGameState = LOADING_SCREEN;
            clearScreen = true;
        }
    }
}

// entry point for program
int main() {

    // Initiialize Joystick
    createHelperEntity(0, 10, 0, 1, 0, CharacterSymbols.symbolSelector);
    selectorTracker[0] = 10; // sets inital X position so it shows up without having to first move

    // Set key system variables to initial values
    currentGameState = LOADING_SCREEN;
    currentDirection = NEUTRAL;
    REFRESH_RATE = 800;
    clearScreen = true;
    swordLocation = 0;
    srand(time(NULL));
    chosenMap = rand()%4;
    bossFight = false;
    fightMode = false;
    victory = false;
    preFightPos = {0 , 0};

    // Startup threads for secondary display
    secondaryLCDThread.start(secondaryLCDThreadFn);

    // Game Loop
    while (true) {

        movementSystem();
        primaryRenderSystem();
        stateManager();

        // controls how often game loop is called
        thread_sleep_for(REFRESH_RATE);        
    }
}