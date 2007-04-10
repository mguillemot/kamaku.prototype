#ifndef __ENEMY
#define __ENEMY

#include <list>
#include "bullet.h"
#include "bordee.h"
#include "gamecolor.h"

class Sprite;

class Enemy
{
public:
    Enemy();
    ~Enemy();
    virtual void update() = 0;
    Bullet *fire_at(int x, int y, float speed, float shift_angle, int bulletType);
    Sprite &sprite();
    void explode();
    bool killed();
    int killedTime();
    void set_difficulty(int diff);
    int difficulty();
    bool collides_with(Bullet *b);
    virtual void hit_by(int damage);
	GameColor getColor() { return _color; }
	void setColor(GameColor c);

protected:
    int _turret_x, _turret_y;
    Sprite *_sprite;
    bool _killed;
    int _killedTime;
    int _difficulty;
    int _life;
	GameColor _color;
};

class RotatingEnemy : public Enemy
{
public:
    RotatingEnemy();
    virtual void update();

protected:
    int _fireCount;
    bool phase;
    int w;
    float shift, acc;
};

class TargetEnemy : public Enemy
{
public:
    TargetEnemy();
    virtual void update();

protected:
    int _fireCount;
};

class SpaceInvaderEnemy : public Enemy
{
public:
    SpaceInvaderEnemy();
    virtual void update();

protected:
    int _direction;
    int _moving;
};

class SilentEnemy : public Enemy
{
public:
    SilentEnemy();
    virtual void update();
	void fire_bordee_at(int x, int y, float speed, float shift_angle, int bulletType, int bulletNbr);

protected:
	std::list<Bordee> _bordees;
};

class RandomColorTarget : public Enemy
{
public:
    RandomColorTarget();
    virtual void update();
};

/*
class Boss : public Enemy
{
public:
    virtual void update();
}
*/

#endif
