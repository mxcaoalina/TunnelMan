#include "Actor.h"
#include "StudentWorld.h"

using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

/********** ACTOR BASE **********/

Actor::Actor(int x, int y, bool isVisible, int imageID, Direction dir, double size, unsigned int depth, unsigned int health, StudentWorld* world)
    : GraphObject(imageID, x, y, dir, size, depth)
    , m_isDead(false)
    , m_health(health)
    , m_world(world)
{
    setVisible(isVisible);
};


Actor::~Actor() {
    //cerr << "Destroying Actor at x: " << getX() << " | y: " << getY() << endl;
}

bool Actor::setDead() {
    if (m_isDead) return false;

    m_isDead = true;
    setVisible(false);
    return true;
}

bool Actor::decrHealthBy(const int n) {
    m_health -= n;
    return true;
}

/********** TunnelMan **********/

TunnelMan::TunnelMan(StudentWorld* world)
    : Actor(TUNNELMAN_SC, TUNNELMAN_SR, true, TID_PLAYER, right, 1.0, 0, TUNNELMAN_TH, world)
    , m_water_count(5)
    , m_sonar_count(1)
    , m_gold_count(0)
{};

TunnelMan::~TunnelMan()
{}

bool TunnelMan::decrHealthBy(const int n) {
    if (getHealth() - n > 0) {
        Actor::decrHealthBy(n);
        return true;
    }
    else {
        Actor::decrHealthBy(n);
        setDead();
        return false;
    }
}


void TunnelMan::handleInput() {
    int ch;
    if (getWorld()->getKey(ch) == true) {
        int p_x = getX();
        int p_y = getY();
        bool isBlocked = false;

        switch (ch) {
        case 'Z':
        case 'z':
            if (m_sonar_count > 0) {
                getWorld()->revealActorsAt(p_x, p_y, 12);
                m_sonar_count--;
            }
            break;
        case KEY_PRESS_TAB:
            dropGold(p_x, p_y);
            break;
        case KEY_PRESS_SPACE:
            createNewSquirt(p_x, p_y, getDirection());
            break;
        case KEY_PRESS_ESCAPE:
            setDead();
            break;
        case KEY_PRESS_LEFT:
            isBlocked = getWorld()->hasBlockingActorAt(p_x - 1, p_y);
            if (p_x - 1 >= 0 && getDirection() == left && !isBlocked)
                moveTo(--p_x, p_y);
            else if (!isBlocked)
                moveTo(p_x, p_y);
            setDirection(left);
            break;
        case KEY_PRESS_RIGHT:
            isBlocked = getWorld()->hasBlockingActorAt(p_x + 1, p_y);
            if (p_x + 1 <= ACTOR_MAXC && getDirection() == right && !isBlocked)
                moveTo(++p_x, p_y);
            else if (!isBlocked)
                moveTo(p_x, p_y);
            setDirection(right);
            break;
        case KEY_PRESS_DOWN:
            isBlocked = getWorld()->hasBlockingActorAt(p_x, p_y - 1);
            if (p_y - 1 >= 0 && getDirection() == down && !isBlocked)
                moveTo(p_x, --p_y);
            else if (!isBlocked)
                moveTo(p_x, p_y);
            setDirection(down);
            break;
        case KEY_PRESS_UP:
            isBlocked = getWorld()->hasBlockingActorAt(p_x, p_y + 1);
            if (p_y + 1 <= ACTOR_MAXR && getDirection() == up && !isBlocked)
                moveTo(p_x, ++p_y);
            else if (!isBlocked)
                moveTo(p_x, p_y);
            setDirection(up);
            break;
        }

        //cerr << "Player moved to x:" << p_x << " | y: " << p_y << endl;
    }

}

/**
 Drops a gold at the specified location and direction
 @param int x, int y
 */
void TunnelMan::dropGold(int x, int y) {
    if (m_gold_count > 0) {
        m_gold_count--;
        Actor* g = new Gold(x, y, true, getWorld());
        getWorld()->addActor(g);
    }
}


/**
 Creates a new squirt at the specified location and direction
 @param int x, int y, int direction
 */
void TunnelMan::createNewSquirt(int x, int y, Direction dir) {
    if (m_water_count > 0) {
        m_water_count--;
        getWorld()->playSound(SOUND_PLAYER_SQUIRT);
        switch (dir) {
        case right:
            x += 4;
            break;
        case left:
            x -= 4;
            break;
        case down:
            y -= 4;
            break;
        case up:
            y += 4;
            break;
        }
        if (getWorld()->isAccessible(x, y, dir)) {
            Actor* s = new Squirt(x, y, dir, getWorld());
            getWorld()->addActor(s);
        }
    }
}


void TunnelMan::doSomething() {
    if (isDead()) return;

    if (getWorld()->removeEarth(getX(), getY(), getX() + 3, getY() + 3)) {
        getWorld()->playSound(SOUND_DIG);
    }

    handleInput();
}

/********** Earth **********/

/**
 Constructor for Earth
 @param int x, int y
 */
Earth::Earth(int x, int y, StudentWorld* world)
    : Actor(x, y, true, TID_EARTH, right, 0.25, 3, 1, world)
{};

Earth:: ~Earth()
{}
/**
 Earth's action -
 @param int x, int y
 */
void Earth::doSomething() {
}

/********** BOULDER **********/

/**
 Constructor for Boulder
 @param int x, int y
 */
Boulder::Boulder(int x, int y, StudentWorld* world)
    : Actor(x, y, true, TID_BOULDER, down, 1.0, 1, BOULDER_HEALTH, world)
    , m_isFalling(false)
    , m_isWaiting(false)
{};


void Boulder::doSomething() {
    if (isDead()) return;

    //Check if stable
    if (!m_isWaiting && !m_isFalling) {
        if (!getWorld()->isEarthAt(getX(), getY() - 1, getX() + 3, getY() - 1)) {
            m_isWaiting = true;
            return;
        }
        else
            return;
    }

    //Check if Waiting
    if (m_isWaiting) {
        if (getHealth() == 0) {
            m_isFalling = true;
            m_isWaiting = false;
            getWorld()->playSound(SOUND_FALLING_ROCK);
            return;
        }
        else {
            decrHealthBy(1);
            return;
        }
    }

    //Is Falling
    if (m_isFalling) {
        if (getY() == 0 || getWorld()->isEarthAt(getX(), getY() - 1, getX() + 3, getY() - 1) || getWorld()->hasBlockingActorAt(getX(), getY() - 1, this)) {
            setDead();
        }
        else {
            moveTo(getX(), getY() - 1);
            getWorld()->causeDamageToActorsAt(getX(), getY(), 100, true, ISBOULDER);
        }

    }
}

/********** SQUIRT **********/

/**
 Constructor for Squirt
 @param int x, int y, int direction
 */
Squirt::Squirt(int x, int y, Direction dir, StudentWorld* world)
    : Actor(x, y, true, TID_WATER_SPURT, dir, 1.0, 1, SQUIRT_HEALTH, world)
{};

Squirt::~Squirt()
{}

/**
 Squirt's action -
 - Detect collision with actors, check for health
   - exit if yes
 - Detect collision with walls
   - exit?
 */

void Squirt::doSomething() {
    if (getWorld()->causeDamageToActorsAt(getX(), getY(), 2, false, ISSQUIRT) || getHealth() == 0) {
        setDead();
        return;
    }
    else {
        int x = getX(), y = getY();
        switch (getDirection()) {
        case up:
            y++;
            break;
        case down:
            y--;
            break;
        case right:
            x++;
            break;
        case left:
            x--;
            break;
        }
        if (getWorld()->isAccessible(x, y, getDirection())) {
            decrHealthBy(1);
            moveTo(x, y);
        }
        else {
            setDead();
        }
    }
}

///////********** Goodies **********/////////


Goodies::Goodies(int x, int y, int imageID, bool isVisible, bool isPlayerPickup, bool isTemp, int health, StudentWorld* world)
    : Actor(x, y, isVisible, imageID, right, 1.0, 2, health, world)
    , m_player_pickup(isPlayerPickup)
    , m_protestor_pickup(!isPlayerPickup)
    , m_isTemporary(isTemp)
{};


/********** OIL BARREL **********/


Oil::Oil(int x, int y, StudentWorld* world)
    : Goodies(x, y, TID_BARREL, false, true, false, 1, world)
{};

Oil::~Oil()
{}

/**
  Oil's tick
  */
void Oil::doSomething() {
    if (isDead()) return;
    if (!isVisible() && getWorld()->playerNearMe(this, 4)) {
        setVisible(true);
        return;
    }
    if (getWorld()->playerNearMe(this, 3)) {
        setDead();
        getWorld()->playSound(SOUND_FOUND_OIL);
        getWorld()->increaseScore(1000);
        return;
    }
}

/********** GOLD NUGGET **********/

/**
  Constructor for Gold
  @param int x, int y, bool isPlayer
  */
Gold::Gold(int x, int y, bool isPlayer, StudentWorld* world)
    : Goodies(x, y, TID_GOLD, isPlayer, !isPlayer, isPlayer, GOLD_HEALTH, world)
{};

Gold::~Gold()
{}
/**
  Gold's tick
  */
void Gold::doSomething() {
    if (isDead()) return;
    if (!isVisible() && getWorld()->playerNearMe(this, 4)) {
        setVisible(true);
        return;
    }
    if (canBePickedUpByPlayer() && getWorld()->playerNearMe(this, 3)) {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(10);
        getWorld()->getTunnelMan()->increaseGold();

        return;
    }
    Actor* protestor = nullptr;
    if (canBePickedUpByProtestor() && getWorld()->protestorNearMe(this, protestor, 3)) {
        setDead();
        getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
        getWorld()->increaseScore(25);
        protestor->pickedUpGold();

        return;
    }
    if (isTemporary()) {
        if (getHealth() == 0) {
            setDead();
            return;
        }
        decrHealthBy(1);
    }

}

/********** SONAR **********/
/**
  Constructor for Sonar
  @param int x, int y
  */
Sonar::Sonar(int x, int y, StudentWorld* world)
    : Goodies(x, y, TID_SONAR, true, true, true, ((300 - 10 * world->getLevel()) > 100 ? (300 - 10 * world->getLevel()) : 100), world)
{}
Sonar::~Sonar()
{}

/**
  Tick handling for Sonar
  */
void Sonar::doSomething() {
    if (isDead()) return;
    if (getWorld()->playerNearMe(this, 3)) {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(75);
        getWorld()->getTunnelMan()->increaseSonar();
        return;
    }
    if (getHealth() == 0) {
        setDead();
        return;
    }
    decrHealthBy(1);
}

/********** WATER **********/
/**
  Constructor for Water
  @param int x, int y
  */
Water::Water(int x, int y, StudentWorld* world)
    : Goodies(x, y, TID_WATER_POOL, true, true, true, ((300 - 10 * world->getLevel()) > 100 ? (300 - 10 * world->getLevel()) : 100), world)
{}

Water::~Water()
{}
/**
  Tick handling for Water
  */
void Water::doSomething() {
    if (isDead()) return;
    if (getWorld()->playerNearMe(this, 3)) {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(100);
        getWorld()->getTunnelMan()->increaseWater();
        return;
    }
    if (getHealth() == 0) {
        setDead();
        return;
    }
    decrHealthBy(1);
}

///////********** PROTESTORS **********/////////

/**
  Constructor for Generic Protestor
  @param int x, int y
  */
GenericProtestor::GenericProtestor(int image, int squaresToMove, int health, StudentWorld* world)
    : Actor(60, 60, true, image, left, 1.0, 0, health, world)
    , m_isLeavingField(false)
    , m_ticksToNextMove(0)
    , m_tickDelay(((3 - world->getLevel() / 4) > 0 ? (3 - world->getLevel() / 4) : 0))
    , m_ticksSinceLastShout(15)
    , m_ticksSinceLastPerpendicular(200)
    , m_noSquaresToMoveInCurrentDir(squaresToMove)
{}

/**
  Handles the move counter for the protestor
  @return true if it's time to move, false otherwise
  */
bool GenericProtestor::shouldIMove() {
    if (m_ticksToNextMove == 0) {
        m_ticksToNextMove = m_tickDelay;
        return true;
    }
    m_ticksToNextMove--;
    return false;
}

void GenericProtestor::doSomething() {
    if (isDead()) return;
    if (!shouldIMove()) return;

    m_ticksSinceLastShout++;
    m_ticksSinceLastPerpendicular++;

    if (isLeaving()) {
        if (getX() == ACTOR_MAXC && getY() == ACTOR_MAXR) {
            setDead();
            return;
        }
        else {
            int nx, ny;
            Direction dir;
            //getWorld()->getNextPosition(getX(), getY(), nx, ny, dir);
            getWorld()->moveTowardsExit(this, nx, ny, dir);
            setDirection(dir);
            moveTo(nx, ny);
            return;
        }
    }

    if (getWorld()->playerNearMe(this, 4)) {
        if (m_ticksSinceLastShout < 15) return;
        bool faceCorrect = false;
        int p_x = getWorld()->getTunnelMan()->getX();
        int p_y = getWorld()->getTunnelMan()->getY();
        switch (getDirection()) {
        case right:
            faceCorrect = p_x > getX();
            break;
        case left:
            faceCorrect = getX() > p_x;
            break;
        case up:
            faceCorrect = getY() < p_y;
            break;
        case down:
            faceCorrect = getY() > p_x;
            break;
        }
        if (faceCorrect) {
            getWorld()->playSound(SOUND_PROTESTER_YELL);
            getWorld()->getTunnelMan()->decrHealthBy(2);
            m_ticksSinceLastShout = 0;
            return;
        }
    }

    if (doPathfinding())
        return;

    Direction dir;
    if (getWorld()->isPlayerWithinSight(getX(), getY(), dir)) {
        setDirection(dir);
        switch (dir) {
        case right:
            moveTo(getX() + 1, getY());
            break;
        case left:
            moveTo(getX() - 1, getY());
            break;
        case up:
            moveTo(getX(), getY() + 1);
            break;
        case down:
            moveTo(getX(), getY() - 1);
            break;
        }
        m_noSquaresToMoveInCurrentDir = 0;
        return;
    }

    if (--m_noSquaresToMoveInCurrentDir <= 0) {
        Direction d = randomDirection();
        for (;;) {
            d = randomDirection();
            bool ok = false;
            switch (d) {
            case right:
                ok = getWorld()->isAccessible(getX() + 4, getY(), d);
                break;
            case left:
                ok = getWorld()->isAccessible(getX() - 1, getY(), d);
                break;
            case up:
                ok = getWorld()->isAccessible(getX(), getY() + 4, d);
                break;
            case down:
                ok = getWorld()->isAccessible(getX(), getY() - 1, d);
                break;
            }
            if (ok) break;
        }
        cerr << "Setting direction to " << d << endl;
        setDirection(d);
        m_noSquaresToMoveInCurrentDir = randInt(8, 60);
    }


    if (m_ticksSinceLastPerpendicular >= 200) {
        StudentWorld* w = getWorld();
        int x = getX(), y = getY();
        switch (getDirection()) {
        case left:
        case right:
        {
            bool canUp = w->isAccessible(x, y + 4, up);
            bool canDown = w->isAccessible(x, y - 1, down);

            if (canUp && canDown) {
                if (randInt(1, 2) == 1) {
                    canUp = false;
                }
                else {
                    canDown = false;
                }
            }

            if (canUp) {
                cerr << "SET DIR UP" << endl;
                setDirection(up);
                m_noSquaresToMoveInCurrentDir = randInt(8, 60);
                m_ticksSinceLastPerpendicular = 0;
            }
            else if (canDown) {
                cerr << "SET DIR DOWN" << endl;
                setDirection(down);
                m_noSquaresToMoveInCurrentDir = randInt(8, 60);
                m_ticksSinceLastPerpendicular = 0;
            }
            break;
        }
        case up:
        case down:
        {
            bool canLeft = w->isAccessible(x - 1, y, left);
            bool canRight = w->isAccessible(x + 4, y, right);
            if (canLeft && canRight) {
                if (randInt(1, 2) == 1) {
                    canLeft = false;
                }
                else {
                    canRight = false;
                }
            }

            if (canLeft) {
                cerr << "SET DIR LEFT" << endl;
                setDirection(left);
                m_noSquaresToMoveInCurrentDir = randInt(8, 60);
                m_ticksSinceLastPerpendicular = 0;
            }
            else if (canRight) {
                cerr << "SET DIR RIGHT" << endl;
                setDirection(right);
                m_noSquaresToMoveInCurrentDir = randInt(8, 60);
                m_ticksSinceLastPerpendicular = 0;
            }
            break;
        }
        }
    }

    Direction d = getDirection();
    switch (d) {
    case left:
        if (getWorld()->isAccessible(getX() - 1, getY(), d)) {
            moveTo(getX() - 1, getY());
            return;
        }
        break;
    case right:
        if (getWorld()->isAccessible(getX() + 4, getY(), d)) {
            moveTo(getX() + 1, getY());
            return;
        }
        break;
    case up:
        if (getWorld()->isAccessible(getX(), getY() + 4, d)) {
            moveTo(getX(), getY() + 1);
            return;
        }
        break;
    case down:
        if (getWorld()->isAccessible(getX(), getY() - 1, d)) {
            moveTo(getX(), getY() - 1);
            return;
        }
        break;
    }

    m_noSquaresToMoveInCurrentDir = 0;
    return;

}

/**
  Decreases the Protestor's healthy by n
  @return true if the decrement does not empty the health
 */
bool GenericProtestor::decrHealthBy(const int n) {
    if (getHealth() - n > 0) {
        Actor::decrHealthBy(n);
        getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
        m_ticksToNextMove = 50 > (100 - getWorld()->getLevel() * 10) ? 50 : (100 - getWorld()->getLevel() * 10);
        return true;
    }
    else {
        Actor::decrHealthBy(n);
        getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
        m_isLeavingField = true;
        m_ticksToNextMove = 0;
        return false;
    }
}

/**
  Constructor for regular protestor
  */
RegularProtestor::RegularProtestor(StudentWorld* world)
    : GenericProtestor(TID_PROTESTER, randInt(8, 64), 5, world)
{};

/**
  What to do when the regular protestor picks up gold
  */
void RegularProtestor::pickedUpGold() {
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(25);
    setLeaving();
}

/**
  Constructor for hardcore protestor
  */
HardcoreProtestor::HardcoreProtestor(StudentWorld* world)
    : GenericProtestor(TID_HARD_CORE_PROTESTER, randInt(8, 64), 20, world)
{};

/**
  What to do when a hardcore protestor picks up gold
  */
void HardcoreProtestor::pickedUpGold() {
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(50);
    int t = 50 > (100 - getWorld()->getLevel() * 10) ? 50 : (100 - getWorld()->getLevel() * 10);
    setTicksToNextMove(t);
}

/**
  Hardcore protestor's pathfinding
  */
bool HardcoreProtestor::doPathfinding() {
    if (getWorld()->playerNearMe(this, 4)) return false;
    if (getWorld()->stepsToPlayer(getX(), getY()) > (16 + getWorld()->getLevel() * 2)) return false;
    int nx, ny;
    Direction dir;
    getWorld()->moveTowardsPlayer(this, nx, ny, dir);
    setDirection(dir);
    moveTo(nx, ny);
    return true;
}


/** Utility Functions **/
Actor::Direction randomDirection() {
    int r = randInt(1, 4);
    switch (r) {
    case 1:
        return Actor::up;
        break;
    case 2:
        return Actor::down;
        break;
    case 3:
        return Actor::left;
        break;
    case 4:
        return Actor::right;
        break;
    }
    return Actor::up;
}



/* Inlined Functions */
inline
bool Actor::isDead() const {
    return m_isDead;
}

inline
int Actor::getHealth() const {
    return m_health;
}

inline
bool Actor::incrHealthBy(const int n) {
    m_health += n;
    return true;
}

inline
bool Actor::isStaticObject() const {
    return true;
}

inline
bool Actor::isUnpassable() const {
    return false;
}

inline
bool Actor::canBePickedUpByPlayer() const {
    return false;
}

inline
bool Actor::canBePickedUpByProtestor() const {
    return false;
}

inline
void Actor::pickedUpGold() {
    return;
}


StudentWorld* Actor::getWorld() const {
    return m_world;
}


int TunnelMan::getHealthAsPct() const {
    return (static_cast<double>(getHealth()) / TUNNELMAN_TH) * 100;
}


int TunnelMan::getWater() const {
    return m_water_count;
}


int TunnelMan::getSonar() const {
    return m_sonar_count;
}


int TunnelMan::getGold() const {
    return m_gold_count;
}


bool TunnelMan::isStaticObject() const {
    return false;
}


int TunnelMan::getType() const {
    return ISTUNNELMAN;
}


void TunnelMan::increaseGold() {
    m_gold_count++;
}


void TunnelMan::increaseSonar() {
    m_sonar_count++;
}


void TunnelMan::increaseWater() {
    m_water_count += 5;
}

inline
int Earth::getType() const {
    return ISEARTH;
}

inline
int Boulder::getType() const {
    return ISBOULDER;
}

inline
int Oil::getType() const {
    return ISOIL;
}

inline
bool Boulder::isUnpassable() const {
    return true;
}

inline
int Squirt::getType() const {
    return ISSQUIRT;
}

inline
int Gold::getType() const {
    return ISGOLD;
}

inline
int Sonar::getType() const {
    return ISSONAR;
}

inline
int Water::getType() const {
    return ISWATER;
}

inline
bool Squirt::isUnpassable() const {
    return false;
}

inline
bool Goodies::canBePickedUpByPlayer() const {
    return m_player_pickup;
}

inline
bool Goodies::canBePickedUpByProtestor() const {
    return m_protestor_pickup;
}

inline
bool Goodies::isTemporary() const {
    return m_isTemporary;
}

inline
bool GenericProtestor::isLeaving() const {
    return m_isLeavingField;
}

inline
bool GenericProtestor::isStaticObject() const {
    return false;
}

inline
void GenericProtestor::setLeaving() {
    m_isLeavingField = true;
}

inline
void GenericProtestor::setTicksToNextMove(int n) {
    m_ticksToNextMove = n;
}

inline
int RegularProtestor::getType() const {
    return ISPROTESTOR;
}

inline
bool RegularProtestor::doPathfinding() {
    return false;
}

inline
int HardcoreProtestor::getType() const {
    return ISHARDCOREPROTESTOR;
}
