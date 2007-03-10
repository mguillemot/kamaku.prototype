#include <math.h>
#include <list>
#include <iostream>
#include "enemy.h"
#include "sprite.h"

extern Sprite *vaisseau;
extern std::list<Bullet *> enemyBullets;

Enemy::Enemy()
{
    SDL_Rect hitbox = {4, 13, 30, 15};
    _sprite = new Sprite("ennemi.bmp");
    _sprite->add_status("ennemi_exploding.bmp");
    _sprite->set_hitbox(hitbox);
    _turret_x = _sprite->position().w / 2;
    _turret_y = _sprite->position().h / 2;
    _killed = false;
    _killedTime = 0;
    _life = 100;
}

RotatingEnemy::RotatingEnemy()
{
    _fireCount = 0;
    _life = 1000;
    phase = true;
    w = 2;
    acc = 0.0;
    shift = 0.0;
}

Enemy::~Enemy()
{
    delete _sprite;
}

void RotatingEnemy::update()
{
    Enemy::update();
    if (!_killed)
    {
        _fireCount++;
        /*
        if (_fireCount == 1)
        {
            _fireCount = 0;
            fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 10, 0.03 * w + shift);
            fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 10, -0.03 * w + shift);
            fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 10, 0.06 * w + shift);
            fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 10, -0.06 * w + shift);
            //shift += 0.02;
            if (phase)
            {
                w++;
            }
            else
            {
                w--;
            }
            if (w == 10)
            {
                phase = false;
            }
            else if (w == 2)
            {
                phase = true;
            }
            if (shift >= 0.5)
            {
                shift = 0.0;
            }
        }
        */
        if (_fireCount == 2)
        {
            _fireCount = 0;
            for (float a = 0.0f; a < (float)M_PI * 2; a += 2 * (float)M_PI / 20)
            {
                fire_at(-1, -1, 10, a + shift, 1);
            }
            acc += 0.005f;
            shift += 0.05f + acc;
        }
    }
}

Bullet *Enemy::fire_at(int x, int y, float speed, float shift_angle, int bulletType)
{
    int from_x = sprite().position().x + _turret_x;
    int from_y = sprite().position().y + _turret_y;
    float angle;
    if (x < 0 && y < 0)
    {
        angle = shift_angle;
    }
    else
    {
        int dx = x - from_x;
        int dy = y - from_y;
        if (dx != 0)
        {
            angle = atan((float)dy / (float)dx);
        }
        else if (dy > 0)
        {
            angle = (float)M_PI_2;
        }
        else
        {
            angle = - (float)M_PI_2;
        }
        if (dx < 0)
        {
            angle += (float)M_PI;
        }
        angle += shift_angle;
    }
    Bullet *shot;
    if (bulletType == 0)
    {
        shot = new BouletteRose(std::make_pair<int, int>(from_x, from_y), angle, speed);
    }
    else
    {
        shot = new Laser(std::make_pair<int, int>(from_x, from_y), angle, speed);
    }
    enemyBullets.push_back(shot);
    return shot;
}

Sprite &Enemy::sprite()
{
    return *_sprite;
}

void Enemy::explode()
{
    _killed = true;
    _sprite->set_status(1);
}

TargetEnemy::TargetEnemy()
{
    _fireCount = 0;
}

void Enemy::update()
{
    if (_killed)
    {
        _killedTime++;
    }
}

void TargetEnemy::update()
{
    Enemy::update();
    if (!_killed)
    {
        _fireCount++;
        if (_fireCount == 5)
        {
            _fireCount = 0;
            float speed = 10.0f + (float)_difficulty / 5;
            fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, speed, 0.0, 0);
            if (_difficulty >= 50)
            {
                fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, speed, 0.1f, 0);
                fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, speed, -0.1f, 0);
            }
        }
    }
}

bool Enemy::killed()
{
    return _killed;
}

int Enemy::killedTime()
{
    return _killedTime;
}

void Enemy::set_difficulty(int diff)
{
    _difficulty = diff;
}

int Enemy::difficulty()
{
    return _difficulty;
}

SpaceInvaderEnemy::SpaceInvaderEnemy()
{
    _direction = 0;
    _moving = 0;
}

void SpaceInvaderEnemy::update()
{
    Enemy::update();
    const int NB_SIDESTEPS = 3;
    if (_moving == 0)
    {
        const float speed = 2.0;
        float sx;
        float sy;
        if (_direction < NB_SIDESTEPS)
        {
            sx = speed;
            sy = 0.0;
        }
        else if (_direction < NB_SIDESTEPS + 1)
        {
            sx = 0.0;
            sy = speed;
        }
        else if (_direction < 2 * NB_SIDESTEPS + 1)
        {
            sx = - speed;
            sy = 0.0;
        }
        else
        {
            sx = 0.0;
            sy = speed;
        }
        sprite().set_speed(sx, sy);
    }
    else if (_moving == 20)
    {
        sprite().set_speed(0.0, 0.0);
    }
    else if (_moving == 50)
    {
        _moving = -1;
        _direction++;
        if (_direction == 2 * (NB_SIDESTEPS + 1))
        {
            _direction = 0;
        }
    }
    _moving++;
    if (!_killed)
    {
        if (rand() % 50 == 1)
        {
            fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 10.0, 0.0, 0);
        }
    }
}

bool Enemy::collides_with(Bullet *b)
{
    if (_killed)
    {
        return false;
    }
    else
    {
        return Sprite::intersects(sprite().hitbox(), b->sprite().hitbox());
    }
}

void Enemy::hit_by(int damage)
{
    _life -= damage;
    if (_life <= 0)
    {
        explode();
    }
}

/*
void Boss::update()
{
    Enemy::update();
}
*/

SilentEnemy::SilentEnemy()
{
	_life = 100000;
}

void SilentEnemy::fire_bordee_at(int x, int y, float speed, float shift_angle, int bulletType, int bulletNbr)
{
	Bordee b;
	b.x = x;
	b.y = y;
	b.speed = speed;
	b.angle = shift_angle;
	b.bullet_type = bulletType;
	b.n = bulletNbr;
	_bordees.push_back(b);
}

void SilentEnemy::update()
{
	Enemy::update();
	std::list<Bordee>::iterator b = _bordees.begin();
	while (b != _bordees.end())
	{
		int i, j;
		j = (b->n <= 5) ? b->n : 5;
		b->n -= j;
		while (j > 0)
		{
			j--;
			Bullet *bullet = fire_at(b->x, b->y, 3.0f, b->angle, b->bullet_type);
			for (i = 0; i < j; i++)
			{
				bullet->update();
			}
			bullet->set_speed(b->speed);
		}
		if (b->n == 0)
		{
			b = _bordees.erase(b);
		}
		else
		{
			b++;
		}
	}
}
