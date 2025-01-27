#ifndef ACTOR_H
#define ACTOR_H

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;

// Type Consts
const int ISBOULDER = 11;
const int ISTUNNELMAN = 12;
const int ISSQUIRT = 13;
const int ISPROTESTOR = 14;
const int ISHARDCOREPROTESTOR = 15;
const int ISEARTH = 16;
const int ISOIL = 17;
const int ISGOLD = 18;
const int ISSONAR = 19;
const int ISWATER = 20;

// Constants
const int TUNNELMAN_SC = 30;
const int TUNNELMAN_SR = 60;
const int TUNNELMAN_TH = 10;
const int ACTOR_MAXC = 60;
const int ACTOR_MAXR = 60;
const int BOULDER_HEALTH = 30;
const int SQUIRT_HEALTH = 4;
const int GOLD_HEALTH = 100;
const int PROTESTOR_SHOUT_TIME = 15;

// Base Actor
class Actor : public GraphObject {
public:
    Actor(int x, int y, bool isVisible, int imageID, Direction dir, double size, unsigned int depth, unsigned int health, StudentWorld* world);
    virtual ~Actor();

    virtual void doSomething() = 0;

    virtual bool isStaticObject() const;
    virtual bool isUnpassable() const;
    virtual bool canBePickedUpByPlayer() const;
    virtual bool canBePickedUpByProtestor() const;

    //accessors
    bool isDead() const;
    int  getHealth() const;
    virtual int getType() const = 0;
    StudentWorld* getWorld() const;

    //mutators
    bool setDead();
    bool incrHealthBy(const int n);
    virtual bool decrHealthBy(const int n);
    virtual void pickedUpGold();

private:
    bool          m_isDead;
    int           m_health;
    StudentWorld* m_world;
};

// TUNNELMAN
class TunnelMan : public Actor {
public:
    TunnelMan(StudentWorld* world);
    ~TunnelMan();

    virtual void doSomething();
    virtual bool isStaticObject() const;

    int getWater() const;
    int getSonar() const;
    int getGold() const;
    int getHealthAsPct() const;
    virtual int getType() const;

    void increaseGold();
    void increaseSonar();
    void increaseWater();

    virtual bool decrHealthBy(const int n);

private:
    unsigned int m_water_count;
    unsigned int m_sonar_count;
    unsigned int m_gold_count;
    void handleInput();
    void createNewSquirt(int x, int y, Direction dir);
    void dropGold(int x, int y);
};

// Earth
class Earth : public Actor {
public:
    Earth(int x, int y, StudentWorld* world);
    ~Earth();
    virtual int getType() const;
    virtual void doSomething();
};

// Boulder
class Boulder : public Actor {
public:
    Boulder(int x, int y, StudentWorld* world);

    virtual int getType() const;
    virtual void doSomething();
    virtual bool isUnpassable() const;
private:
    bool m_isFalling;
    bool m_isWaiting;
};

// Squirt
class Squirt : public Actor {
public:
    Squirt(int x, int y, Direction dir, StudentWorld* world);
    ~Squirt();
    virtual int getType() const;
    virtual void doSomething();
    virtual bool isUnpassable() const;
};

// Goodies
class Goodies : public Actor {
public:
    Goodies(int x, int y, int imageID, bool isVisible, bool isPlayerPickup, bool isTemp, int health, StudentWorld* world);
    virtual bool canBePickedUpByPlayer() const;
    virtual bool canBePickedUpByProtestor() const;
    bool isTemporary() const;

private:
    bool m_player_pickup;
    bool m_protestor_pickup;
    bool m_isTemporary;
};

// Oil
class Oil : public Goodies {
public:
    Oil(int x, int y, StudentWorld* world);
    ~Oil();
    virtual void doSomething();
    virtual int getType() const;
};

// Gold
class Gold : public Goodies {
public:
    Gold(int x, int y, bool isPlayer, StudentWorld* world);
    ~Gold();
    virtual void doSomething();
    virtual int getType() const;
};

// Sonar
class Sonar : public Goodies {
public:
    Sonar(int x, int y, StudentWorld* world);
    ~Sonar();
    virtual void doSomething();
    virtual int getType() const;
};

// Water
class Water : public Goodies {
public:
    Water(int x, int y, StudentWorld* world);
    ~Water();
    virtual void doSomething();
    virtual int getType() const;
};

// Protestor
class GenericProtestor : public Actor {
public:
    GenericProtestor(int image, int squaresToMove, int health, StudentWorld* world);
    virtual void doSomething();
    bool isLeaving() const;
    virtual int getType() const = 0;
    virtual bool doPathfinding() = 0;
    virtual bool decrHealthBy(const int n);
    virtual bool isStaticObject() const;
    void setLeaving();
    void setTicksToNextMove(int n);

private:
    bool shouldIMove();
    bool m_isLeavingField;
    const int m_tickDelay;
    int m_ticksToNextMove;
    int m_ticksSinceLastShout;
    int m_ticksSinceLastPerpendicular;
    int m_noSquaresToMoveInCurrentDir;
};

// Regular Protestor
class RegularProtestor : public GenericProtestor {
public:
    RegularProtestor(StudentWorld* world);
    virtual int getType() const;
    virtual bool doPathfinding();
    virtual void pickedUpGold();
};

// Hardcore Protestor
class HardcoreProtestor : public GenericProtestor {
public:
    HardcoreProtestor(StudentWorld* world);
    virtual int getType() const;
    virtual bool doPathfinding();
    virtual void pickedUpGold();
};


Actor::Direction randomDirection();
#endif // ACTOR_H_