#include <math.h>
#include "bullet.h"

Bullet::Bullet(std::pair<int, int> position, float direction, float speed)
{
/*
    _direction = direction;
    _speed = speed;
    update_sprite_speed();
*/
}

Bullet::~Bullet()
{
    delete _sprite;
}

void Bullet::update_sprite_speed()
{
    float sx = _speed * cos(_direction);
    float sy = _speed * sin(_direction);
    _sprite->set_speed(sx, sy);
}

void Bullet::update()
{
    _sprite->update_position();
}

Sprite &Bullet::sprite()
{
    return *_sprite;
}

float Bullet::speed()
{
    return _speed;
}

float Bullet::direction()
{
    return _direction;
}

void Bullet::set_speed(float speed)
{
    _speed = speed;
    update_sprite_speed();
}

void Bullet::set_direction(float direction)
{
    _direction = direction;
    update_sprite_speed();
}

Laser::Laser(std::pair<int, int> position, float direction, float speed) : Bullet(position, direction, speed)
{
    _sprite = new Sprite("laser.bmp");
    SDL_Rect laserHitbox = {1, 1, 2, 12};
    _sprite->set_hitbox(laserHitbox);
    _sprite->set_rotation(direction);
    _sprite->set_position(position.first - _sprite->position().w / 2, position.second - _sprite->position().h / 2);
    _direction = direction;
    set_speed(speed);
}

ColoredBullet::ColoredBullet(std::pair<int, int> position, float direction, float speed, GameColor c) : Bullet(position, direction, speed)
{
	switch (c)
	{
	case Red:
		_sprite = new Sprite("boulette_rouge.bmp");
		break;
	case Green:
		_sprite = new Sprite("boulette_verte.bmp");
		break;
	case Blue:
		_sprite = new Sprite("boulette_bleue.bmp");
		break;
	default:
		_sprite = new Sprite("boulette.bmp");
		break;
	}
    SDL_Rect bouletteHitbox = {2, 2, 4, 4};
    _sprite->set_hitbox(bouletteHitbox);
    _sprite->set_position(position.first - _sprite->position().w / 2, position.second - _sprite->position().h / 2);
    _direction = direction;
    set_speed(speed);
}
