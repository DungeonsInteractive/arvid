#include "mbed.h"
#include <string>
#include <iostream>
using std::string;


class player {       
 protected:
    string pClass;        
    int pHealth, pMana, pAtk, pDef, pLives;
    char customChar;
    
    // character sprites

    player( string c, int h, int a, int d, int l){
        pClass = c;
        pHealth = h;
        pAtk = a;
        pDef = d;
        pLives = l;
    }

    public:
    // Setters
    void setpHealth(int x) {
      pHealth = x;
    }
    // Getters
    int getpHealth() {
      return pHealth;
    }
    // do same for others 

    //methods to be created:

    //attack
    //abilities
    //movement ?  
    //defend
    //potions 
    //
};

class enemy {
  protected:
    string eClass;        
    int eHealth, eMana, eAtk, eDef, eLives;     

    // character sprites

    enemy( string c, int m, int a, int d, int l) {
        eClass = c;
        eMana = m;
        eAtk = a;
        eDef = d;
        eLives = l;
    }

    public:
    // Setters
    void seteHealth(int x) {
      eHealth = x;
    }
    // Getters
    int geteHealth() {
      return eHealth;
    }
    // do same for others 
};

int main()
{


    while (true) {

    }
}