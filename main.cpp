#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <list>
#include "bullet.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "settings.h"
#include "timer.h"
#include "sprite.h"
#include "star.h"
#include "enemy.h"
#include "sm.h"
#include "gamecolor.h"

#ifdef main
#undef main
#endif

Uint32 hitboxColor, starColor;
std::list<Bullet *> enemyBullets, myBullets;
Sprite *vaisseau = NULL;
bool show_hitbox = false;    
SDL_Surface *screen = NULL;

struct Arrow
{
	int column;
	int time;
};

int main(int argc, char *argv[])
{
    SDL_Surface *message = NULL;
    SDL_Surface *hitCount = NULL;
    SDL_Event event;
    TTF_Font *font = NULL;
    Mix_Music *music = NULL; 
    Mix_Chunk *collision = NULL;
    bool quit = false;
    bool move_up = false;
    bool move_down = false;
    bool move_left = false;
    bool move_right = false;
    std::list<Enemy *> enemies;
    bool sound = false;
    int frame = 0;
    Timer fps;
    int hits = 0;
    SDL_Color textColor = {255, 255, 255};
    Uint32 bgColor, laserYellow, laserLightblue, laserPurple;
    std::list<Star *> stars;
    bool fireLaser = false, fireShot = false, fireBomb = false;
	bool button1 = false, button2 = false, button3 = false;
    SDL_Rect fireRect;
    int destroyed = 0;
	GameColor shotColor = None;
	GameColor laserColor = None;
     
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    { 
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }
    
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "Couldn't set required video mode: %s\n",
                        SDL_GetError());
        exit(1);
    }
    SDL_WM_SetCaption("Kamaku", NULL);
    
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    music = Mix_LoadMUS("justice.wav");
    collision = Mix_LoadWAV("hit.wav");
    
    TTF_Init();
    font = TTF_OpenFont("times.ttf", 28);
    if (font == NULL)
    {
        fprintf(stderr, "Couldn't open font: TIMES.TTF\n");
        exit(1);
    }
    
    SDL_Rect vaisseauHitbox = {10, 11, 8, 9};
    SDL_Rect bouletteHitbox = {2, 2, 4, 4};
    vaisseau = new Sprite("vaisseau.bmp");
    vaisseau->set_hitbox(vaisseauHitbox);
    vaisseau->set_position(SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT - 80);
    for (int i = 0; i < NB_STARS; i++)
    {
        stars.push_back(new Star());
    }
    
    starColor = SDL_MapRGB(screen->format, 0xAF, 0xAF, 0xAF);
    bgColor = SDL_MapRGB(screen->format, 0, 0, 0);
    hitboxColor = SDL_MapRGB(screen->format, 0, 0xFF, 0);
    laserYellow = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0);
    laserLightblue = SDL_MapRGB(screen->format, 0xFF, 0, 0xFF);
    laserPurple = SDL_MapRGB(screen->format, 0, 0xFF, 0xFF);
    hitCount = TTF_RenderText_Solid(font, "Not hit yet!", textColor);
  
	/*
    for (int jj = 0; jj < 4; jj++)
    {
        for (int ii = 0; ii < 7; ii++)
        {
            Enemy *e = new SpaceInvaderEnemy();
            e->sprite().set_position(10 + ii * 50, 10 + jj * 42);
            enemies.push_back(e);
        }
    }
	*/
	SilentEnemy *migi = new SilentEnemy();
	migi->sprite().set_position(SCREEN_WIDTH - 60, 40);
	//enemies.push_back(migi);
	SilentEnemy *hidari = new SilentEnemy();
	hidari->sprite().set_position(30, 40);
	//enemies.push_back(hidari);

    if (Mix_PlayingMusic() == 0)
    { 
        Mix_PlayMusic(music, -1);
        Mix_PauseMusic();
    }

	SMfile sm;
	load_sm(&sm, "justice.sm");
	SDL_Rect noteRect[4];
	noteRect[0].x = 50;
	noteRect[0].w = 50;
	noteRect[0].y = 50;
	noteRect[0].h = 50;
	noteRect[1].x = 150;
	noteRect[1].w = 50;
	noteRect[1].y = 50;
	noteRect[1].h = 50;
	noteRect[2].x = 250;
	noteRect[2].w = 50;
	noteRect[2].y = 50;
	noteRect[2].h = 50;
	noteRect[3].x = 350;
	noteRect[3].w = 50;
	noteRect[3].y = 50;
	noteRect[3].h = 50;
	Uint32 beatColor = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	Uint32 beatBgColor = SDL_MapRGB(screen->format, 0x40, 0x40, 0x40);
	Uint32 beatFgColor = SDL_MapRGB(screen->format, 0x80, 0x80, 0x80);
	Timer beatTimer;
	int offset = (int)(sm.offset * 1000);
	int start_at = 1000 + offset;
	float bpm = 60000.0f / sm.bpms[0].bpm;
	Level level = sm.level[3];
	int consumed = 0;
	std::list<Arrow> arrows;
	int m;
	float time = (float)start_at; // pkoi faut-il ajouter le décalage musical 2x ??
	for (m = 0; m < level.measure_nb; m++)
	{
		int n;
		for (n = 0; n < level.measure[m].notes_nb; n++)
		{
			int t;
			for (t = 0; t < 4; t++)
			{
				if (level.measure[m].notes[n].tap[t] == TAP)
				{
					Arrow a;
					a.column = t;
					a.time = (int)time;
					arrows.push_back(a);
				}
			}
			time += 4 * bpm / level.measure[m].notes_nb;
		}
	}
    Mix_ResumeMusic();
	beatTimer.start();

    while (quit == false)
    {
        fps.start();
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_UP:
                        move_up = true;
                        break;
                    case SDLK_DOWN:
                        move_down = true;
                        break;
                    case SDLK_LEFT:
                        move_left = true;
                        break;
                    case SDLK_RIGHT:
                        move_right = true;
                        break;
                    case SDLK_h:
                        show_hitbox = !show_hitbox;
                        break;
                    case SDLK_m:
                        if (Mix_PausedMusic())
                        {
                            Mix_ResumeMusic();
                        }
                        else
                        {
                            Mix_PauseMusic();
                        }
                        break;
                    case SDLK_l:
                        sound = !sound;
                        break;
                    case SDLK_s:
                        button1 = true;
                        break;
                    case SDLK_d:
                        button2 = true;
                        break;
					case SDLK_f:
						button3 = true;
						break;
                }                         
            }
            else if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_UP:
                        move_up = false;
                        break;
                    case SDLK_DOWN:
                        move_down = false;
                        break;
                    case SDLK_LEFT:
                        move_left = false;
                        break;
                    case SDLK_RIGHT:
                        move_right = false;
                        break;
                    case SDLK_s:
                        button1 = false;
                        break;
                    case SDLK_d:
                        button2 = false;
                        break;
					case SDLK_f:
						button3 = false;
						break;
                }                         
            }
            else if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }
		if (button1 && button2 && button3)
		{
			fireShot = false;
			fireLaser = false;
			fireBomb = true;
		}
		else if (button1 && button2)
		{
			fireShot = false;
			fireLaser = true;
			laserColor = Yellow;
			fireBomb = false;
		}
		else if (button1 && button3)
		{
			fireShot = false;
			fireLaser = true;
			laserColor = Purple;
			fireBomb = false;
		}
		else if (button2 && button3)
		{
			fireShot = false;
			fireLaser = true;
			laserColor = Lightblue;
			fireBomb = false;
		}
		else if (button1)
		{
			fireShot = true;
			shotColor = Red;
			fireLaser = false;
			fireBomb = false;
		}
		else if (button2)
		{
			fireShot = true;
			shotColor = Green;
			fireLaser = false;
			fireBomb = false;
		}
		else if (button3)
		{
			fireShot = true;
			shotColor = Blue;
			fireLaser = false;
			fireBomb = false;
		}
		else
		{
			fireShot = false;
			fireLaser = false;
			fireBomb = false;
		}
                
        SDL_FillRect(screen, NULL, bgColor);
        for (std::list<Star *>::iterator i = stars.begin(); i != stars.end(); i++)
        {
            (*i)->display(screen);
            (*i)->update();
        }

        float sx = 0;
        float sy = 0;
        float sp;
        if (fireLaser)
        {
            sp = SHIP_FIRE_SPEED;
        }
        else
        {
            sp = SHIP_NORMAL_SPEED;
        }
        if (move_up)
        {
            sy -= sp;
        }
        if (move_down)
        {
            sy += sp;
        }
        if (move_left)
        {
            sx -= sp;
        }
        if (move_right)
        {
            sx += sp;
        }
        vaisseau->set_speed(sx, sy);
        vaisseau->update_position();
        vaisseau->display(screen);

		if (fireLaser)
		{
			fireRect.x = vaisseau->position().x - 5 + vaisseau->position().w / 2;
			fireRect.y = 0;
			fireRect.w = 10;
			fireRect.h = vaisseau->position().y;
		}

        if (destroyed == 28)
        {
            destroyed = 0;
            Enemy *e = new RotatingEnemy();
            e->sprite().set_position(rand() % 380 + 50, -20);
            e->sprite().set_speed(0.0f, 2.0f);
            enemies.push_back(e);
        }

		if ((frame % 120) == 0)
		{
			Enemy *e = new RandomColorTarget();
            e->sprite().set_position(rand() % 140 + 50, -20);
            e->sprite().set_speed(0.0f, 0.8f);
            enemies.push_back(e);
		}

        Enemy *hitByLaser = NULL;
        std::list<Enemy *>::iterator j = enemies.begin();
        while (j != enemies.end())
        {
            Enemy *e = *j;
            e->update();
            if (e->killed() && e->killedTime() == 20)
            {
                delete e;
                j = enemies.erase(j);
                destroyed++;
            }
            else
            {
                e->sprite().update_position();
                if (fireLaser)
                {
                    if (Sprite::intersects(fireRect, e->sprite().hitbox()) && !e->killed())
                    {
                        int lower = e->sprite().position().y + e->sprite().position().h;
                        int newHeight = fireRect.y + fireRect.h - lower;
                        fireRect.y = lower;
                        fireRect.h = newHeight;
                        hitByLaser = e;
                    }
                }
                e->sprite().display(screen);
                j++;
            }
        }
        if (fireLaser)
        {
			Uint32 c = 0;
			if (laserColor == Yellow)
			{
				c = laserYellow;
			}
			else if (laserColor == Lightblue)
			{
				c = laserLightblue;
			}
			else
			{
				c = laserPurple;
			}
            SDL_FillRect(screen, &fireRect, c);
            if (hitByLaser != NULL)
            {
                hitByLaser->hit_by(LASER_STRENGTH);
            }
        }

		/*
        SDL_Rect blackHole;
        blackHole.x = 200;
        blackHole.y = 400;
        blackHole.w = 100;
        blackHole.h = 100;
        SDL_FillRect(screen, &blackHole, fireColor);
		*/

        std::list<Bullet *>::iterator i = enemyBullets.begin();
        while (i != enemyBullets.end())
        {
            Bullet *b = *i;
            b->update();
            if (b->sprite().dead())
            {
                delete b;
                i = enemyBullets.erase(i);
            }
            else
            {
                b->sprite().display(screen);
                /*
				if (Sprite::intersects(blackHole, b->sprite().hitbox()))
                {
                    b->set_direction(b->direction() + 0.1f);
                }
				*/
                if (vaisseau->collides_with(&b->sprite()))
                {
                    hits++;
                    std::string hitTxt = "Hits: ";
                    std::string nbOfHits;
                    std::stringstream ss;
                    ss << hits;
                    ss >> nbOfHits;
                    hitTxt += nbOfHits;
                    SDL_FreeSurface(hitCount);
                    hitCount = TTF_RenderText_Solid(font, hitTxt.c_str(), textColor);
                    if (sound)
                    {
                        Mix_PlayChannel(-1, collision, 0);
                    }
                    delete b;
                    i = enemyBullets.erase(i);
                }
                else
                {
                    i++;
                }
            }
        }
        Sprite::apply_surface(5, 40, hitCount, screen);

        std::list<Bullet *>::iterator k = myBullets.begin();
        while (k != myBullets.end())
        {
            Bullet *b = *k;
            b->update();
            if (b->sprite().dead())
            {
                delete b;
                k = myBullets.erase(k);
            }
            else
            {
                b->sprite().display(screen);
                bool impact = false;
                for (std::list<Enemy *>::iterator kk = enemies.begin(); kk != enemies.end(); kk++)
                {
                    Enemy *e = *kk;
                    if (e->collides_with(b))
                    {
                        e->hit_by(BULLET_STRENGTH);
                        delete b;
                        k = myBullets.erase(k);
                        impact = true;
                        break;
                    }
                }
                if (!impact)
                {
                    k++;
                }
            }
        }

		/*
		if (beatTimer.get_ticks() - consumed >= (int)bpm)
		{
			consumed += (int)bpm;
			SDL_FillRect(screen, &noteRect[0], beatFgColor);
			SDL_FillRect(screen, &noteRect[1], beatFgColor);
			SDL_FillRect(screen, &noteRect[2], beatFgColor);
			SDL_FillRect(screen, &noteRect[3], beatFgColor);
		}
		else
		{
			SDL_FillRect(screen, &noteRect[0], beatBgColor);
			SDL_FillRect(screen, &noteRect[1], beatBgColor);
			SDL_FillRect(screen, &noteRect[2], beatBgColor);
			SDL_FillRect(screen, &noteRect[3], beatBgColor);
		}
		*/

		while (!arrows.empty() && beatTimer.get_ticks() >= arrows.front().time)
		{
			if (arrows.front().column == LEFT_A)
			{
				// laser
			}
			else if (arrows.front().column == UP_A)
			{
				if (fireShot)
				{
					Bullet *b1 = new ColoredBullet(Sprite::center(vaisseau->position()), (float)M_PI + (float)M_PI_2, 20.0f, shotColor);
					Bullet *b2 = new ColoredBullet(Sprite::center(vaisseau->position()), (float)M_PI + (float)M_PI_2 + 0.1f, 20.0f, shotColor);
					Bullet *b3 = new ColoredBullet(Sprite::center(vaisseau->position()), (float)M_PI + (float)M_PI_2 - 0.1f, 20.0f, shotColor);
					myBullets.push_back(b1);
					myBullets.push_back(b2);
					myBullets.push_back(b3);
				}			
			}
			/*
			if (arrows.front().column == LEFT_A)
			{
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.0f, 0, 40);
			}
			else if (arrows.front().column == RIGHT_A)
			{
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.0f, 0, 40);
			}
			else if (arrows.front().column == UP_A)
			{
				migi->fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 15.0f, 0.0f, 1);
				hidari->fire_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 15.0f, 0.0f, 1);
			}
			else if (arrows.front().column == DOWN_A)
			{
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.1f, 0, 40);
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.2f, 0, 40);
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.3f, 0, 40);
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.4f, 0, 40);
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.1f, 0, 40);
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.2f, 0, 40);
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.3f, 0, 40);
				hidari->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.4f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.1f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.2f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.3f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, 0.4f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.1f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.2f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.3f, 0, 40);
				migi->fire_bordee_at(vaisseau->position().x + vaisseau->position().w / 2, vaisseau->position().y + vaisseau->position().h / 2, 20.0f, -0.4f, 0, 40);
			}
			*/
			arrows.pop_front();
		}

		/*
		int now = beatTimer.get_ticks();
		for (std::list<Arrow>::iterator arrow_iter = arrows.begin(); arrow_iter != arrows.end(); arrow_iter++)
		{
			int future = (*arrow_iter).time - now;
			int pos = 50 + future / 3;
			if (pos >= 0 && pos < SCREEN_HEIGHT)
			{
				SDL_Rect r = noteRect[(*arrow_iter).column];
				r.y = pos;
				SDL_FillRect(screen, &r, beatColor);
			}
		}
		*/

        std::string nbFrame;
        std::stringstream frameSs;
        frameSs << frame;
        frameSs >> nbFrame;
        std::string frameTxt = "Frame #" + nbFrame;
        SDL_FreeSurface(message);
        message = TTF_RenderText_Solid(font, frameTxt.c_str(), textColor);
        Sprite::apply_surface(5, 5, message, screen);
        SDL_Flip(screen);
        frame++;

        while (fps.get_ticks() < 1000 / FRAMES_PER_SECOND)
        { 
            SDL_Delay(1); 
        }
    }
    
    for (std::list<Bullet *>::iterator i = enemyBullets.begin(); i != enemyBullets.end(); i++)
    {
        delete *i;
    }
    for (std::list<Bullet *>::iterator i = myBullets.begin(); i != myBullets.end(); i++)
    {
        delete *i;
    }
    for (std::list<Enemy *>::iterator i = enemies.begin(); i != enemies.end(); i++)
    {
        delete *i;
    }    
    for (std::list<Star *>::iterator i = stars.begin(); i != stars.end(); i++)
    {
        delete *i;
    }    
    Mix_FreeMusic(music);
    Mix_FreeChunk(collision);
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);
    TTF_Quit();
    Mix_CloseAudio();
    SDL_Quit();
        
    printf("Quiting...\n");
    //system("PAUSE");
    return 0;
}
