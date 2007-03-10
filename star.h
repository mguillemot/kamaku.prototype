#ifndef __STAR
#define __STAR

#include "SDL.h"

class Star
{
public:
    Star();
    void update();
    void display(SDL_Surface *screen);
  
private:
    int _x;
    float _y;
    float _speed;
    
    void recreate();
};

#endif
