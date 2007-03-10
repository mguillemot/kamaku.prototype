#ifndef __SPRITE
#define __SPRITE

#include <string>
#include <utility>
#include <vector>
#include "SDL.h"

class Sprite
{
public:
    static void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination);
    static bool intersects(SDL_Rect a, SDL_Rect b);
    static std::pair<int, int> center(SDL_Rect r);

    Sprite(std::string filename);
    ~Sprite();
    void display(SDL_Surface *screen);
    void set_speed(float sx, float sy);
    void set_position(int x, int y);
    void set_hitbox(SDL_Rect hitbox);
    void update_position();
    SDL_Rect position();
    SDL_Rect hitbox();
    bool collides_with(Sprite *s);
    bool dead();
    void die();
    bool out_of_screen();
    float sx();
    float sy();
    void set_status(int status);
    int status();
    void add_status(std::string filename);
    float rotation();
    void set_rotation(float rotation);

private:
    static SDL_Surface *read_bmp(std::string filename);

    SDL_Surface *current_surface();

    std::vector<SDL_Surface *> _surfaces;
    SDL_Rect _position, _hitbox;
    float _x, _y;
    float _sx, _sy; 
    bool _dead;
    int _status;
    float _rotation;
};

#endif
