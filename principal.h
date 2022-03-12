#ifndef PRINCIPAL_H_INCLUDED
#define PRINCIPAL_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define STATUS_STATE_LIVES 0
#define STATUS_STATE_GAME 1
#define STATUS_STATE_GAMEOVER 2

#define MAN_HEIGHT 60.0
#define MAN_WIDTH_STANDING 13.0
#define MAN_WIDTH_MOVING 35.0

typedef struct
{
    double x, y, w, h;
    double dx, dy;
    short lives;
    char *name;
    //flags
    int onLedge;
    int animFrame;
    int facingRight;
    int slowingdown;
    int isCentered;
    int isDead;
}Man;

typedef struct
{
    int x,y;
}Ball;

typedef struct
{
    double x,y,w,h;
}Ledge;

typedef struct
{
    double x, y, w, h;
    double dx;
}Spike;

typedef enum {SPIKE, FIREBALL} DeathCause;

typedef struct
{
    Man man;    //players

    Ball balls[100];    //enemies

    Ledge ledges[100]; //platforms

    Spike spike;

    //Images:
    SDL_Texture *spikes;
    SDL_Texture *angryBall;
    SDL_Texture *manFrames[4];
    SDL_Texture *deathFrames[2];
    SDL_Texture *brick;
    SDL_Texture *label;
    int labelW, labelH;

    //renderer
    SDL_Renderer *renderer;

    //Fonts:
    TTF_Font *font;

    int time, deathCountdown;   //time
    int statusState; //game status
    DeathCause deathCause;

    double scrollX;
    double spikeProgress;
}GameState;


#endif // PRINCIPAL_H_INCLUDED
