#include <iostream>
#include <stdexcept>
#include <math.h>
#include "sprite.h"
#include "settings.h"
//#include "sge.h"

extern bool show_hitbox;
extern Uint32 hitboxColor;

SDL_Surface *Sprite::read_bmp(std::string filename)
{
    SDL_Surface *loadedImage;
    SDL_Surface *optimizedImage;

    /* Load the BMP file into a surface */
    loadedImage = SDL_LoadBMP(filename.c_str());
    if (loadedImage == NULL)
    {
        fprintf(stderr, "Couldn't load %s: %s\n", filename.c_str(), SDL_GetError());
        return NULL;
    }
    
    optimizedImage = SDL_DisplayFormat(loadedImage);
    SDL_FreeSurface(loadedImage);
    
    if (optimizedImage != NULL)
    {
        Uint32 colorkey = SDL_MapRGB(optimizedImage->format, 0xFF, 0, 0xFF);
        SDL_SetColorKey(optimizedImage, SDL_RLEACCEL | SDL_SRCCOLORKEY, colorkey);
    }

    return optimizedImage;
}

void Sprite::apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination)
{ 
    SDL_Rect offset;
    offset.x = x; 
    offset.y = y;
    SDL_BlitSurface(source, NULL, destination, &offset);
} 

Sprite::Sprite(std::string filename)
{
    add_status(filename);
    _x = 0;
    _y = 0;
    _position.w = _surfaces[0]->w;
    _position.h = _surfaces[0]->h;
    _hitbox.x = 0;
    _hitbox.y = 0;
    _hitbox.w = _surfaces[0]->w;
    _hitbox.h = _surfaces[0]->h;
    _sx = 0;
    _sy = 0;
    _dead = false;
    _status = 0;
    _rotation = 0.0;
}
    
Sprite::~Sprite()
{
    for (std::vector<SDL_Surface *>::iterator i = _surfaces.begin(); i != _surfaces.end(); i++)
    {
        SDL_FreeSurface(*i);
    }
}

void Sprite::display(SDL_Surface *screen)
{
    apply_surface(position().x, position().y, current_surface(), screen);
    if (show_hitbox)
    {
        SDL_Rect h = hitbox();
        SDL_FillRect(screen, &h, hitboxColor);
    }
}
    
void Sprite::set_speed(float sx, float sy)
{
    _sx = sx;
    _sy = sy;
}

void Sprite::set_position(int x, int y)
{
    _x = (float)x;
    _y = (float)y;
}

void Sprite::set_hitbox(SDL_Rect hitbox)
{
    _hitbox = hitbox;
}

void Sprite::update_position()
{
    float new_x = _x + _sx;
    float new_y = _y + _sy;
    if ((new_x + _position.w) < 0 || new_x >= SCREEN_WIDTH ||
        (new_y + _position.h) < 0 || new_y >= SCREEN_HEIGHT)
    {
        _dead = true;
    }
    _x = new_x;
    _y = new_y;
}

SDL_Rect Sprite::position()
{
    _position.x = (int)_x;
    _position.y = (int)_y;
    return _position;
}

SDL_Rect Sprite::hitbox()
{
    SDL_Rect h = _hitbox;
    h.x += position().x;
    h.y += position().y;
    return h;
}

bool Sprite::intersects(SDL_Rect a, SDL_Rect b)
{
    return !(b.x > a.x + a.w || b.x + b.w < a.x || b.y > a.y + a.h || b.y + b.h < a.y);
}

bool Sprite::collides_with(Sprite *s)
{
    return Sprite::intersects(hitbox(), s->hitbox());
}

bool Sprite::dead()
{
    return _dead;
}

void Sprite::die()
{
    _dead = true;
}

bool Sprite::out_of_screen()
{
    SDL_Rect pos = position();
    return (pos.x + pos.w < 0 || pos.x >= SCREEN_WIDTH || pos.y + pos.h < 0 || pos.y >= SCREEN_HEIGHT);
}

float Sprite::sx()
{
    return _sx;
}

float Sprite::sy()
{
    return _sy;
}

std::pair<int, int> Sprite::center(SDL_Rect r)
{
    return std::make_pair<int, int>(r.x + r.w / 2, r.y + r.h / 2);
}

void Sprite::set_status(int status)
{
    _status = status;
}

int Sprite::status()
{
    return _status;
}

void Sprite::add_status(std::string filename)
{
    SDL_Surface *surf = read_bmp(filename);
    if (surf == NULL)
    {
        throw new std::runtime_error("Unable to load sprite BMP.");
    }
    int size = (int)_surfaces.size();
    _surfaces.resize(size + 1);
    _surfaces[size] = surf;
}

SDL_Surface *Sprite::current_surface()
{
    return _surfaces[_status];
}

float Sprite::rotation()
{
    return _rotation;
}

void Sprite::set_rotation(float rotation)
{
    _rotation = rotation;
    Uint32 colorkey = SDL_MapRGB(_surfaces[0]->format, 0xFF, 0, 0xFF);
    float degree = 360.0f / 2 / (float)M_PI * rotation + 90.0f;
	/*
    for (std::vector<SDL_Surface *>::iterator i = _surfaces.begin(); i != _surfaces.end(); i++)
    {
        SDL_Surface *rotatedSurface = sge_transform_surface(*i, colorkey, degree, 1.0, 1.0, SGE_TAA);
        SDL_SetColorKey(rotatedSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY, colorkey);
        SDL_FreeSurface(*i);
        *i = rotatedSurface;
    }
	*/
    SDL_Rect h;
    h.x = _surfaces[0]->w / 2 - 1;
    h.y = _surfaces[0]->h / 2 - 1;
    h.w = 2;
    h.h = 2;
    set_hitbox(h);
    _position.w = _surfaces[0]->w;
    _position.h = _surfaces[0]->h;
}
