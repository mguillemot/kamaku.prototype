#ifndef __BULLET
#define __BULLET

#include <utility>
#include "sprite.h"
#include "gamecolor.h"

class Bullet
{
public:
    ~Bullet();
    void update();
    Sprite &sprite();
    float speed();
    float direction();
    void set_speed(float speed);
    void set_direction(float direction);

protected:
    void update_sprite_speed();
    Bullet(std::pair<int, int> position, float direction, float speed);

    float _direction;
    float _speed;
    Sprite *_sprite;
};

class Laser : public Bullet
{
public:
    Laser(std::pair<int, int> position, float direction, float speed);
};

class ColoredBullet : public Bullet
{
public:
    ColoredBullet(std::pair<int, int> position, float direction, float speed, GameColor c = None);
};

#endif
