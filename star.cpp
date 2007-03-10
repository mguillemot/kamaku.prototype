#include "star.h"
#include "settings.h"

extern Uint32 starColor;

Star::Star()
{
    recreate();
    _y = (float)(rand() % SCREEN_HEIGHT);
}

void Star::update()
{
    _y += _speed;
    if (_y > SCREEN_HEIGHT)
    {
        recreate();
    }
}

void Star::display(SDL_Surface *screen)
{
    SDL_Rect r;
    r.x = _x;
    r.y = (int)_y;
    r.w = 2;
    r.h = 2;
    SDL_FillRect(screen, &r, starColor);        
}

void Star::recreate()
{
    _x = rand() % SCREEN_WIDTH;
    _y = 0.0f;
    _speed = 1.0f + (float)(rand() % 200) / 10.0f;
}
