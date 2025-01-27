#include "StudentWorld.h"
#include "Actor.h"
#include <string>
#include <cstdlib>
#include <cmath>
#include <queue>

using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
    return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp


StudentWorld::StudentWorld(string assetDir)
    : GameWorld(assetDir)
    , m_noOil(0)
    , m_noBoulders(0)
    , m_noNuggets(0)
    , m_noProtestors(0)
    , m_ticksSinceLastProtestor(0)
    , m_totalPossibleProtestors(0)
    , m_hardcoreProtestorProbability(0)
    , m_goodieProbability(0)
{}

/**
 Destructor for StudentWorld
 */
StudentWorld::~StudentWorld() {
    for (int x = 0; x < EARTH_C; x++) {
        for (int y = 0; y < EARTH_R; y++) {
            delete m_earth_arr[x][y];
        }
    }
    delete m_player;
    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++) {
        delete (*it);
    }
}


int StudentWorld::init() {

    //Seeds the random number
    srand(time(0));

    // Paint earth
    for (int y = 0; y < EARTH_R; y++) {
        for (int x = 0; x < EARTH_C; x++) {
            m_earth_arr[x][y] = new Earth(x, y, this);
            //Mine shaft
            if (y >= SHAFT_RS && y <= SHAFT_RE && x >= SHAFT_CS && x <= SHAFT_CE)
                m_earth_arr[x][y]->setDead();
        }
    }

    // Distribute Actors
    m_noBoulders = (getLevel() / 2 + 2) > 6 ? 6 : (getLevel() / 2 + 2);
    m_noNuggets = (5 - getLevel() / 2) > 2 ? (5 - getLevel() / 2) : 2;
    m_noOil = (getLevel() + 2) > 21 ? 21 : (getLevel() / 2 + 2);

    m_protestorTickDelay = 25 > (200 - getLevel()) ? 25 : (200 - getLevel());
    m_ticksSinceLastProtestor = 0;
    m_totalPossibleProtestors = (2 + getLevel() * 1.5) > 15 ? 15 : (2 + getLevel() * 1.5);
    m_hardcoreProtestorProbability = (getLevel() * 10 + 30) > 90 ? 90 : (getLevel() * 10 + 30);
    m_goodieProbability = getLevel() * 25 + 300;

    // Distribute Boulders
    for (int i = 0; i < m_noBoulders; ++i) {
        int x = 0, y = 0;
        possibleDistributedPos(x, y, true);
        Actor* b = new Boulder(x, y, this);
        m_actors.push_back(b);
        removeEarth(x, y, x + 3, y + 3);
    }

    // Distribute Oil
    for (int i = 0; i < m_noOil; ++i) {
        int x = 0, y = 0;
        possibleDistributedPos(x, y, false);
        Actor* o = new Oil(x, y, this);
        m_actors.push_back(o);
    }

    // Distribute Gold
    for (int i = 0; i < m_noNuggets; ++i) {
        int x = 0, y = 0;
        possibleDistributedPos(x, y, false);
        Actor* g = new Gold(x, y, false, this);
        m_actors.push_back(g);
    }

    // New Player
    m_player = new TunnelMan(this);

    return GWSTATUS_CONTINUE_GAME;
}


int StudentWorld::move()
{
    setDisplayText();
    m_player->doSomething();

    //player has moved, update heat maps
    updateHeatMap(m_player->getX(), m_player->getY(), m_toPlayerHeatMap);
    updateHeatMap(ACTOR_MAXC, ACTOR_MAXR, m_toExitHeatMap);

    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++) {
        (*it)->doSomething();
    }

    if (m_ticksSinceLastProtestor > 0) {
        m_ticksSinceLastProtestor--;
    }
    else if (m_noProtestors < m_totalPossibleProtestors) {
        m_ticksSinceLastProtestor = m_protestorTickDelay;
        if (randInt(1, m_hardcoreProtestorProbability) == 1) {
            Actor* p = new HardcoreProtestor(this);
            m_actors.push_back(p);
        }
        else {
            Actor* p = new RegularProtestor(this);
            m_actors.push_back(p);
        }
        m_noProtestors++;
    }

    if (randInt(1, m_goodieProbability) == 1) {
        if (randInt(1, 5) == 1) {
            Actor* s = new Sonar(0, 60, this);
            m_actors.push_back(s);
        }
            int x = 0, y = 0, tries = 100;
            for (;;) {
                //This hack prevents this from potentially running forever
                //The game is given 100 tries to find an empty spot, if that fails
                //it'll just dump it somewhere in the shaft.
                if (tries == 0) {
                    x = SHAFT_CS;
                    y = randInt(0, DISTRIBUTED_ER);
                    break;
                }
                possibleDistributedPos(x, y, false);
                if (isEarthAt(x, y, x + 3, y + 3)) {
                    tries--;
                    continue;
                }
                else break;
            }
            Actor* w = new Water(x, y, this);
            m_actors.push_back(w);
        
    }



    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end();) {
        if ((*it)->isDead()) {
            if ((*it)->getType() == ISOIL) {
                m_noOil--;
            }
            if ((*it)->getType() == ISPROTESTOR || (*it)->getType() == ISHARDCOREPROTESTOR) {
                m_noProtestors--;
            }
            delete (*it);
            it = m_actors.erase(it);
        }
        else
            it++;
    }

    if (m_player->isDead()) {
        decLives();
        m_noProtestors = 0;
        m_ticksSinceLastProtestor = 0;
        return GWSTATUS_PLAYER_DIED;
    }

    if (m_noOil == 0) {
        m_noProtestors = 0;
        m_ticksSinceLastProtestor = 0;
        return GWSTATUS_FINISHED_LEVEL;
    }

    return GWSTATUS_CONTINUE_GAME;
}



void StudentWorld::cleanUp()
{
    for (int x = 0; x < EARTH_C; x++) {
        for (int y = 0; y < EARTH_R; y++) {
            delete m_earth_arr[x][y];
        }
    }

    for (list<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); it++) {
        delete (*it);
    }

    m_actors.clear();

    delete m_player;
}


bool StudentWorld::addActor(Actor* a) {

    m_actors.push_back(a);
    return true;
}



bool StudentWorld::actorsAreClose(Actor* a, Actor* b, int limit) const {
    return limit >= calculateDistance(a->getX(), a->getY(), b->getX(), b->getY());
}


bool StudentWorld::actorsAreClose(const int x, const int y, Actor* b, int limit) const {
    return limit >= calculateDistance(x, y, b->getX(), b->getY());
}



bool StudentWorld::removeEarth(const int x, const int y) {
    if (x < EARTH_C && y < EARTH_R)
        return m_earth_arr[x][y]->setDead();
    return false;
}


bool StudentWorld::removeEarth(const int sx, const int sy, const int ex, const int ey) {
    bool hasRemovedEarth = false;
    for (int x = sx; x <= ex; x++) {
        for (int y = sy; y <= ey; y++) {
            if (removeEarth(x, y))
                hasRemovedEarth = true;
        }
    }
    return hasRemovedEarth;
};


bool StudentWorld::isEarthAt(const int sx, const int sy, const int ex, const int ey) const {
    if (sx > ex || sy > ey || sx < 0 || sy < 0) {
        return false;
    }

    for (int x = sx; x <= ex; ++x) {
        if (x >= EARTH_C) continue;
        for (int y = sy; y <= ey; ++y) {
            if (y >= EARTH_R) continue;
            if (!m_earth_arr[x][y]->isDead()) {
                return true;
            }
        }
    }
  
    return false;
}


bool StudentWorld::isAccessible(int x, int y, Actor::Direction dir) const {
    if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT) return false;
    int yEndCheck = y;
    int xEndCheck = x;
    switch (dir) {
    case Actor::right:
    case Actor::left:
        yEndCheck += 3;
        break;
    case Actor::down:
    case Actor::up:
        xEndCheck += 3;
        break;
    }
   
    return (!isEarthAt(x, y, xEndCheck, yEndCheck) && !hasBlockingActorAt(x, y));
}



bool StudentWorld::playerNearMe(Actor* me, int limit) const {
    return actorsAreClose(me, m_player, limit);
}


void StudentWorld::revealActorsAt(int x, int y, int limit) {
    for (list<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if (actorsAreClose(x, y, (*it), limit)) {
            if (!(*it)->isVisible() && !(*it)->isDead())
                (*it)->setVisible(true);
        }
    }
}

bool StudentWorld::protestorNearMe(Actor* me, Actor*& other, int limit) const {
    for (list<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); it++) {
        int type = (*it)->getType();
        if (type == ISPROTESTOR || type == ISHARDCOREPROTESTOR) {
            if (actorsAreClose(me, (*it), limit)) {
                other = (*it);
                return true;
            }
        }
    }
    return false;
}


bool StudentWorld::hasBlockingActorAt(const int x, const int y) const {
    for (list<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if (!(*it)->isUnpassable()) continue;
        if (actorsAreClose(x, y, (*it), 3)) return true;
    }
    return false;
}


bool StudentWorld::hasBlockingActorAt(const int x, const int y, const Actor* a) const {
    for (list<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if (!(*it)->isUnpassable() || (*it) == a) continue;
        if (actorsAreClose(x, y, (*it), 3)) return true;
    }
    return false;
}


bool StudentWorld::causeDamageToActorsAt(const int x, const int y, int damage, bool hitsPlayer, int damageCausingType) {
    bool hasHitSomething = false;
    for (list<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->isStaticObject()) continue;
        if (actorsAreClose(x, y, (*it), 3)) {
            cerr << "A protestor is hit!" << endl;
            if (!(*it)->decrHealthBy(damage)) {
                //has killed actor

                if (damageCausingType == ISBOULDER) {
                    increaseScore(500);
                }
                else if (damageCausingType == ISSQUIRT) {
                    if ((*it)->getType() == ISPROTESTOR) {
                        //normal protestor
                        increaseScore(100);
                    }
                    else if ((*it)->getType() == ISHARDCOREPROTESTOR) {
                        //hardcore
                        increaseScore(250);
                    }
                }
            }
            hasHitSomething = true;
        }
    }

    if (hitsPlayer && actorsAreClose(x, y, m_player, 3)) {
        hasHitSomething = true;
        if (damage >= m_player->getHealth())
            m_player->setDead();
        else
            m_player->decrHealthBy(damage);
    }
    return hasHitSomething;
}


void StudentWorld::possibleDistributedPos(int& x, int& y, bool isBoulder) const {
    int startX = isBoulder ? BOULDER_SR : 0;
    int sx = 0;
    int sy = 0;

    for (;;) {
        sx = randInt(0, DISTRIBUTED_EC);
        sy = randInt(startX, DISTRIBUTED_ER);

        if (!(sx < SHAFT_CS - 4 || sx > SHAFT_CE)) continue;

        bool isTooClose = false;
        for (list<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); it++) {
            isTooClose = calculateDistance((*it)->getX(), (*it)->getY(), sx, sy) < DISTRIBUTED_LIMIT;
            if (isTooClose) break;
        }

        if (!isTooClose) {
            x = sx;
            y = sy;
            //cerr << "Distributing Actor at x:" <<  x << " | y:" << y << endl;
            return;
        }
    }
}


bool StudentWorld::isPlayerWithinSight(int x, int y, Actor::Direction& dir) const {
    if (m_player->getX() == x) {
        //UP
        for (int i = y; i <= ACTOR_MAXR; i++) {
            if (!isAccessible(x, i, Actor::up)) return false;
            if (m_player->getY() == i) {
                //cerr << "Protestor sees player above" << endl;
                dir = Actor::up;
                return true;
            }
        }
        //DOWN
        for (int i = y; i >= 0; i--) {
            if (!isAccessible(x, i, Actor::down)) return false;
            if (m_player->getY() == i) {
                //cerr << "Protestor sees player below" << endl;
                dir = Actor::down;
                return true;
            }
        }
    }
    else if (m_player->getY() == y) {
        //RIGHT
        for (int i = x; i <= ACTOR_MAXC; i++) {
            if (!isAccessible(i, y, Actor::right)) return false;
            if (m_player->getX() == i) {
                //cerr << "Protestor sees player right" << endl;
                dir = Actor::right;
                return true;
            }
        }
        //LEFT
        for (int i = x; i >= 0; i--) {
            if (!isAccessible(i, y, Actor::left)) return false;
            if (m_player->getX() == i) {
                //cerr << "Protestor sees player left" << endl;
                dir = Actor::left;
                return true;
            }
        }
    }

    return false;

}

void StudentWorld::exploreMapPos(int x, int y, int origin, Actor::Direction dir, queue<P>& q, int m[][VIEW_HEIGHT]) {
    if (m[x][y] == UNSEEN) {
        switch (dir) {
        case Actor::left:
            if (!isAccessible(x, y, dir)) {
                m[x][y] = BLOCKED;
                return;
            }
            break;
        case Actor::right:
            if (!isAccessible(x + 3, y, dir)) {
                m[x][y] = BLOCKED;
                return;
            }
            break;
        case Actor::up:
            if (!isAccessible(x, y + 3, dir)) {
                m[x][y] = BLOCKED;
                return;
            }
            break;
        case Actor::down:
            if (!isAccessible(x, y, dir)) {
                m[x][y] = BLOCKED;
                return;
            }
            break;
        }
        m[x][y] = origin;
        q.push(P(x, y, origin));
    }
}


/**
  Updates heat map
  */
void StudentWorld::updateHeatMap(int destx, int desty, int map[][VIEW_HEIGHT]) {

    for (int i = 0; i < VIEW_WIDTH; i++) {
        for (int j = 0; j < VIEW_HEIGHT; j++)
            map[i][j] = UNSEEN;
    }

    queue<P> q;

    q.push(P(destx, desty, 0));
    map[destx][desty] = 0;

    while (!q.empty()) {
        P p = q.front();
        q.pop();

        //up
        exploreMapPos(p.x, p.y + 1, p.originNum + 1, Actor::up, q, map);
        //down
        exploreMapPos(p.x, p.y - 1, p.originNum + 1, Actor::down, q, map);
        //left
        exploreMapPos(p.x - 1, p.y, p.originNum + 1, Actor::left, q, map);
        //right
        exploreMapPos(p.x + 1, p.y, p.originNum + 1, Actor::right, q, map);
    }
}

void StudentWorld::moveTowardsPlayer(Actor* a, int& nx, int& ny, Actor::Direction& dir) {
    int x = a->getX(), y = a->getY();

    int bestValue = BLOCKED;
    //up
    if (m_toPlayerHeatMap[x][y + 1] < bestValue) {
        bestValue = m_toPlayerHeatMap[x][y + 1];
        nx = x;
        ny = y + 1;
        dir = Actor::up;
    }
    //down
    if (m_toPlayerHeatMap[x][y - 1] < bestValue) {
        bestValue = m_toPlayerHeatMap[x][y - 1];
        nx = x;
        ny = y - 1;
        dir = Actor::down;
    }
    //left
    if (m_toPlayerHeatMap[x - 1][y] < bestValue) {
        bestValue = m_toPlayerHeatMap[x - 1][y];
        nx = x - 1;
        ny = y;
        dir = Actor::left;
    }
    //right
    if (m_toPlayerHeatMap[x + 1][y] < bestValue) {
        bestValue = m_toPlayerHeatMap[x + 1][y];
        nx = x + 1;
        ny = y;
        dir = Actor::right;
    }

}

void StudentWorld::moveTowardsExit(Actor* a, int& nx, int& ny, Actor::Direction& dir) {
    int x = a->getX(), y = a->getY();

    int bestValue = BLOCKED;
    //up
    if (m_toExitHeatMap[x][y + 1] < bestValue) {
        bestValue = m_toExitHeatMap[x][y + 1];
        nx = x;
        ny = y + 1;
        dir = Actor::up;
    }
    //down
    if (m_toExitHeatMap[x][y - 1] < bestValue) {
        bestValue = m_toExitHeatMap[x][y - 1];
        nx = x;
        ny = y - 1;
        dir = Actor::down;
    }
    //left
    if (m_toExitHeatMap[x - 1][y] < bestValue) {
        bestValue = m_toExitHeatMap[x - 1][y];
        nx = x - 1;
        ny = y;
        dir = Actor::left;
    }
    //right
    if (m_toExitHeatMap[x + 1][y] < bestValue) {
        bestValue = m_toExitHeatMap[x + 1][y];
        nx = x + 1;
        ny = y;
        dir = Actor::right;
    }
}

int StudentWorld::stepsToPlayer(int x, int y) const {
    return m_toPlayerHeatMap[x][y];
}


/*
string StudentWorld::formatToString(int num, unsigned int finalLength, char filler) {
    string s = to_string(num);
    while (s.length() != finalLength) {
        s.insert(0, 1, filler);
    }
    return s;
}
*/
/**
 Set display text
 */
void StudentWorld::setDisplayText() {
    string separation = " ";

    ostringstream oss;
    oss.setf(ios::fixed);
    oss << "Lvl: " << setw(2) << getLevel() << separation;
    oss << " Lives: " << setw(1) << getLives() << separation;
    oss << "Hlth: " << setw(3) << 10 * m_player->getHealth() << "%" << separation;
    oss << "Wtr: " << setw(2) << m_player->getWater() << separation;
    oss << "Gld: " << setw(2) << m_player->getGold() << separation;
    oss << "Oil Left: " << setw(2) << getOil() << separation;
    oss << "Sonar: " << setw(2) << m_player->getSonar() << separation;
    oss.fill('0');
    oss << "Scr: " << setw(6) << getScore() << "  ";

    setGameStatText(oss.str());
}

/** Utility Functions **/
int randInt(int begin, int end) {
    end++;
    return begin + rand() % (end - begin);
}

double calculateDistance(int sx, int sy, int ex, int ey) {
    return sqrt((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy));
}

// Inline Fuctions
inline
int StudentWorld::getOil() const {
    return m_noOil;
}


TunnelMan* StudentWorld::getTunnelMan() {
    return m_player;
}
