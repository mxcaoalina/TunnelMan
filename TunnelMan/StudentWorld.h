#ifndef STUDENTWORLD_H
#define STUDENTWORLD_H

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"

#include <sstream> // defines the type std::ostringstream
#include <iomanip> // defines the manipulator setw

#include <list>
#include <vector>
#include <queue>

const int EARTH_R = 60;
const int EARTH_C = 64;
const int SHAFT_RS = 4;
const int SHAFT_RE = 59;
const int SHAFT_CS = 30;
const int SHAFT_CE = 33;

const int BOULDER_SR = 20;
const int DISTRIBUTED_EC = 60;
const int DISTRIBUTED_ER = 56;
const int DISTRIBUTED_LIMIT = 6;

const int BLOCKED = 99999;
const int KNOWN = 99991;
const int SEEN = 99911;
const int UNSEEN = 99111;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);
	virtual ~StudentWorld();

	virtual int init();
	virtual int move();
	virtual void cleanUp();
	//overloaded function to remove single squear earth
	bool removeEarth(const int x, const int y);
	//overloaded function to remove an area of earth
	bool removeEarth(const int sx, const int sy, const int ex, const int ey);
	bool isEarthAt(const int sx, const int sy, const int ex, const int ey) const;
	//checks if there's any blocking actors (i.e. boulders) in a small range around a specific position,
//and the overloaded version accepts an additional pointer to exclude that from the checking
	bool hasBlockingActorAt(const int x, const int y) const;
	bool hasBlockingActorAt(const int x, const int y, const Actor* a) const;
	//this does a specified damage to actors in range of a specific coordinate, and
	//can further determine if this affects the Frackman, and what causes the damage
	bool causeDamageToActorsAt(const int x, const int y, int damage, bool hitsPlayer, int damageCausingType);
	//determines if a specified location is walkable, 
	//given an orientation and a coordinate of the FINAL position
	bool isAccessible(int x, int y, Actor::Direction dir) const;
	//adds an actor into the list of actors
	bool addActor(Actor* a);
	//checks if two actors are within a specified perimeter of each other
	bool playerNearMe(Actor* me, int limit) const;
	bool protestorNearMe(Actor* me, Actor*& other, int limit) const;
	//sets all actors in a circle, center x, y and radius limit to visible
	void revealActorsAt(int x, int y, int limit);
	//this returns the next position and orientation for an actor from the pathfinding heatmap
	//directed towards either the player or the exit
	void moveTowardsPlayer(Actor* a, int& nx, int& ny, Actor::Direction& dir);
	void moveTowardsExit(Actor* a, int& nx, int& ny, Actor::Direction& dir);

	int stepsToPlayer(int x, int y) const;
	//determines if, at the specified position, the player is within a direct path of sight
	bool isPlayerWithinSight(int x, int y, Actor::Direction& dir) const;

	TunnelMan* getTunnelMan();

private:
	struct P {
		int x;
		int y;
		int originNum;
		P(int xx, int yy, int n)
			: x(xx)
			, y(yy)
			, originNum(n)
		{};
	};

	std::list<Actor*> m_actors;


	int m_toPlayerHeatMap[VIEW_WIDTH][VIEW_HEIGHT];
	int m_toExitHeatMap[VIEW_WIDTH][VIEW_HEIGHT];

	Earth* m_earth_arr[EARTH_C][EARTH_R];
	TunnelMan* m_player;

	int m_noOil;
	int m_noBoulders;
	int m_noNuggets;
	int m_noProtestors;

	int m_protestorTickDelay;
	int m_ticksSinceLastProtestor;
	int m_totalPossibleProtestors;
	int m_hardcoreProtestorProbability;
	int m_goodieProbability;

	bool actorsAreClose(Actor* a, Actor* b, int limit) const;
	bool actorsAreClose(const int x, const int y, Actor* b, int limit) const;
	void possibleDistributedPos(int& x, int& y, bool isBoulder) const;
	void exploreMapPos(int x, int y, int origin, Actor::Direction dir, std::queue<P>& q, int m[][VIEW_HEIGHT]);
	void setDisplayText();
	void updateHeatMap(int destx, int desty, int map[][VIEW_HEIGHT]);
	int getOil() const;

};


// Utility Functions
int randInt(int begin, int end);
double calculateDistance(int sx, int sy, int ex, int ey);

#endif // STUDENTWORLD_H_