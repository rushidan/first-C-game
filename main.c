/*TO-DO:
    1. Add exploding angry balls with sub particles spreading in hexagonal direction
    2. Add a line of spikes along the left edge of the screen continuously progressing towards the player
*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <principal.h>
#include <status.h>

#define GRAVITY 0.32

int processEvents(SDL_Window* window, GameState* game)
{
    SDL_Event event;
    int done=0;
    while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_WINDOWEVENT_CLOSE:
                {
                    if(window)
                    {
                        SDL_DestroyWindow(window);
                        window=NULL;
                        done=1;
                    }
                }
            break;
            case SDL_KEYDOWN:
                {
                    switch(event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        done=1;
                    break;

                    case SDLK_UP:  //basic or minimum jump
                        if(game->man.onLedge && game->statusState == STATUS_STATE_GAME)
                        {
                            game->man.onLedge = 0;
                            game->man.dy = -7.85;
                        }
                    break;
                    }
                }
            break;
            case SDL_QUIT:
                done=1;
            break;
            }
        }
    if(game->statusState == STATUS_STATE_GAME)
    {
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_RIGHT])
        {
            game->man.facingRight = 1;
            //game->man.x += 10;
            game->man.dx += 1; //acceleration
            game->man.slowingdown = 0;
            if(game->man.dx > 6)
                game->man.dx = 6; // max speed
        }
        else if(state[SDL_SCANCODE_LEFT])
        {
            game->man.facingRight = 0;
            //game->man.x -= 10;
            game->man.dx -= 1; //acceleration
            game->man.slowingdown = 0;
            if(game->man.dx < -6)
                game->man.dx = -6;   //max speed
        }
        else
        {
            //game->man.animFrame = 0;
            //friction
            game->man.slowingdown = 1;
            if(game->man.dx > 0)
                game->man.dx -= 1;
            else if(game->man.dx < 0)
                game->man.dx += 1;
        }
        if(state[SDL_SCANCODE_UP] && game->man.dy<0)
            game->man.dy -= 0.18; //more jump on holding
        /*if(state[SDL_SCANCODE_UP])
            game->man.y -= 10;
        if(state[SDL_SCANCODE_DOWN])
            game->man.y += 10;*/
    }
        return done;
}

void process(GameState *game)
{
    //add time:
    game->time++;

    if(game->time == 120)   //after 120 frames or nearly 2 seconds of loading the game
    {
        shutdown_status_lives(game);
        game->statusState = STATUS_STATE_GAME;
    }

    if(game->statusState == STATUS_STATE_GAME)
    {
        //movement:
        game->man.x += game->man.dx; //walking or running
        game->man.y += game->man.dy; //jumping
        game->spike.x += game->spike.dx;

        //animation:
        if(game->time % 8 == 0) //change animation after every 8 frames
        {
            if(game->man.dx && game->man.onLedge && !game->man.slowingdown)
            {
                if(game->man.animFrame<3)
                    game->man.animFrame++;
                else
                    game->man.animFrame=0;
            }
            else if(game->man.slowingdown && game->man.onLedge)
                game->man.animFrame = 0;
        }

        //falling down from flight:
        if(!game->man.onLedge)
            game->man.dy += GRAVITY; //falling
    }

    if(game->man.isDead)
    {
        game->deathCountdown++;
        if(game->deathCountdown == 180) //after 120 frames or nearly 3 seconds of death
        {
            game->man.lives--;
            resetGame(game);
        }
    }
    //without spike:
    /*static double temp = 320-8; //for storing current position of game->man.x
    if(game->man.dx>0 && game->man.isCentered)
    {
        temp = game->man.x;
        game->scrollX = -game->man.x + (320-8);
    }

    if(game->man.dx<0)
    {
        game->man.isCentered = 0;
        game->scrollX = -temp + (320-8); //to keep scrollX value fixed to the one right before going left
    }
    //320-8 is the centering position for man rectangle
    if((game->man.x + game->scrollX) >= (320-8)) //working with fractions, so >=, otherwise logically ==
        game->man.isCentered = 1;
    //printf("%d\n",game->man.isCentered);*/
}

int collisionDetect(double x1, double y1, double w1, double h1, double x2, double y2, double w2, double h2)  //detects the collision between 2 rectangles
{
    return !((x1 > (x2+w2)) || (x2 > (x1+w1)) || (y1 > (y2+h2)) || (y2 > (y1+h1)));
}

void collisionProcess(GameState *game)
{
    int i;
    static j = 0;
    double mx,my,mw,mh,bx,by,bw,bh;

    //collision with spike
    if((game->spike.x + game->spike.w) >= game->man.x)
    {
        game->man.isDead = 1;
        game->deathCause = SPIKE;
    }
    //collision with ledges
    for(i=0;i<100;i++)
    {
        if(game->man.animFrame == 0 || game->man.animFrame == 2)
        {
            game->man.w = MAN_WIDTH_STANDING;
            game->man.h = MAN_HEIGHT;
        }
        else
        {
            game->man.w = MAN_WIDTH_MOVING;
            game->man.h = MAN_HEIGHT;
        }
        mw=game->man.w;
        mh=game->man.h;
        mx=game->man.x, my=game->man.y;
        bx=game->ledges[i].x, by=game->ledges[i].y, bw=game->ledges[i].w, bh=game->ledges[i].h;

        if((my+mh)>by && my<(by+bh)) //collision in the horizontal direction
        {
            //game->man.onLedge = 0;
            //rubbing against right edge
            if(mx<(bx+bw) && (mx+mw)>(bx+bw))
            {
                //correct x
                game->man.x = bx+bw;
                mx = bx+bw;
                game->man.dx = 0;
            }
            //rubbing against left edge
            if((mx+mw)>bx && mx<bx)
            {
                //correct x
                game->man.x = bx-mw;
                mx = bx-mw;
                game->man.dx = 0;
            }
        }

        if((mx+mw)>bx && mx<(bx+bw)) //collision in the vertical direction
        {
            //landing on ledge
            if((my+mh)>=by && my<by)
            {
                //correct y
                game->man.y = by-mh;
                my = by-mh;
                game->man.dy = 0; //landing on ledge, stop any jump velocity
                game->man.onLedge = 1;
                j=i;
            }
            //bumping head
            else if(my<(by+bh) && my>by)
            {
                //correct y
                game->man.y = by+bh;
                my = by+bh;
                game->man.dy = 0; //bumping head, stop any jump velocity
                game->man.onLedge = 0;
            }
        }

        else if((mx+mw)<game->ledges[j].x || mx>(game->ledges[j].x+bw)) //if goes outside the bounds of the landing ledge, then falls
        {
            game->man.onLedge = 0;
        }

    }
    //printf("%.15lf\n%.15lf\n%.15lf\n%.15lf\n%.15lf\n%.15lf\n%d\n",mx,my,0,0,game->man.dx,game->man.dy,game->man.onLedge);
    //printf("%d\n",game->man.onLedge);
}

void doRender(SDL_Renderer* renderer, GameState* game)
{
    int i;
    static double dmx = 0, dsx = 0; //for error correction
    static double deathX = 0, deathY = 0, deathW = 0, deathH = 0; //for storing the death locations
    //initialization of Rectangles:
    static SDL_Rect manRect = {320-8, 240-30, MAN_WIDTH_STANDING, MAN_HEIGHT};
    static SDL_Rect ledgeRect[100] = {{0}, {0}, {0}, {0}};
    static SDL_Rect spikeRect = {0, 0, 120, 480};
    if(game->time>1)
        goto jump;
    for(i=0;i<100;i++)
    {
        ledgeRect[i].x = game->ledges[i].x;
        ledgeRect[i].y = game->ledges[i].y;
        ledgeRect[i].w = game->ledges[i].w;
        ledgeRect[i].h = game->ledges[i].h;
    }


    jump:
    if(game->statusState == STATUS_STATE_LIVES)
        draw_status_lives(game);
    else if(game->statusState == STATUS_STATE_GAME && !game->man.isDead)
    {
        SDL_SetRenderDrawColor(renderer,128,0,255,50);

        SDL_RenderClear(renderer);

        //MAN:
        if(game->man.animFrame == 0 || game->man.animFrame == 2)
        {
            game->man.w = MAN_WIDTH_STANDING;
            game->man.h = MAN_HEIGHT;
        }
        else
        {
            game->man.w = MAN_WIDTH_MOVING;
            game->man.h = MAN_HEIGHT;
        }
        //error correction
        if(dsx)
            manRect.x -= dsx;
        //process scrolling in x direction
        if((!game->man.dx) && spikeRect.x == 0)
            manRect.x -= game->spike.dx;
        else if(game->man.dx && game->man.dx <= game->spike.dx && spikeRect.x == 0)
            manRect.x -= (game->spike.dx - game->man.dx);
        else if(game->man.dx >= game->spike.dx && manRect.x < 320-8)
        {
            manRect.x += (game->man.dx - game->spike.dx);
            if(manRect.x > 320-8)
            {
                dmx = manRect.x - 312;
                manRect.x = 312;
            }
        }

        //other parameters(if any)
        manRect.y = game->man.y;
        manRect.w = game->man.w;
        manRect.h = game->man.h;
        //process render
        SDL_RenderCopyEx(renderer,game->manFrames[game->man.animFrame],NULL,&manRect,0,NULL,(game->man.facingRight == 0));

        //LEDGE:
        for(i=0; i<100; i++)
        {
            //error correction
            if(dmx || dsx)
            {
                ledgeRect[i].x -= (dmx + dsx);
                dsx = 0;
            }
            //process scrolling in x direction
            if(!game->man.dx && spikeRect.x == 0)
                ledgeRect[i].x -= game->spike.dx;
            else if(game->man.dx && game->man.dx <= game->spike.dx && spikeRect.x == 0)
                ledgeRect[i].x -= game->spike.dx;
            else if(game->man.dx && game->man.dx <= game->spike.dx && spikeRect.x < 0)
                ledgeRect[i].x -= game->man.dx;
            else if(game->man.dx >= game->spike.dx && manRect.x < 320-8)
                ledgeRect[i].x -= game->spike.dx;
            else if(game->man.dx >= game->spike.dx && manRect.x == 320-8)
                ledgeRect[i].x -= game->man.dx;

            //process render
            SDL_RenderCopy(renderer, game->brick, NULL, &ledgeRect[i]);
        }

        //SPIKE:
        //error correction
        if(dmx)
        {
            spikeRect.x -= dmx;
            dmx = 0;
        }
        //process scrolling in x direction
        if(game->man.dx >= game->spike.dx && manRect.x == 320-8)
            spikeRect.x -= (game->man.dx - game->spike.dx);
        else if(game->man.dx <= game->spike.dx && spikeRect.x < 0)
        {
            spikeRect.x += (game->spike.dx - game->man.dx);
            if(spikeRect.x > 0)
            {
                dsx = spikeRect.x;
                spikeRect.x = 0;
            }
        }
        //process render
        SDL_RenderCopy(renderer, game->spikes, NULL, &spikeRect);
    }

    else if(game->man.isDead)
    {
        if(game->deathCause == SPIKE)
        {
            if(!deathX && !deathY && !deathW && !deathH)
            {
                deathX = manRect.x-10;
                deathY = manRect.y;
                deathW = manRect.w + 5;
                deathH = manRect.h;
            }

            SDL_Color currentBackground = {128,0,255,50};
            SDL_RenderFillRect(renderer, &manRect); // erase the death position of man by the background color
            SDL_Rect deathRect = {deathX, deathY, deathW, deathH};
            SDL_RenderCopy(renderer, game->deathFrames[SPIKE], NULL, &deathRect);

            //initialize all the rects again
            manRect.x = 320-8;
            manRect.y = 240-30;
            manRect.w - MAN_WIDTH_STANDING;
            manRect.h = MAN_HEIGHT;
            spikeRect.x = 0;
        }
    }

    //present renderer
    SDL_RenderPresent(renderer);
}

void loadGame(GameState* game)          //game initializer
{
    //load fonts:
    game->font = NULL;
    game->font = TTF_OpenFont("CRUSHED c PERSONAL USE ONLY.ttf",48);
    if(!game->font)
    {
        printf("Cannot find font file\n");
        SDL_Quit();
        exit(1);
    }

    game->label = NULL;

    SDL_Surface *surface = NULL;

    game->man.isDead = 0;
    game->man.lives = 3;

    game->man.x = 320-8; //for centering : man.x = (window width/2) - (rectangle width/2)
    game->man.y = 240-30; //for centering : man.y = (window height/2) = (rectangle height/2)
    game->man.dx = 0;
    game->man.dy = 0;

    game->spike.x = 0;
    game->spike.y = 0;
    game->spike.w = 120;
    game->spike.h = 480;
    game->spike.dx = 3;

    //flags
    game->man.onLedge = 0;
    game->man.animFrame = 0;
    game->man.facingRight = 1;
    game->man.slowingdown = 0;
    game->scrollX = 0.0;
    game->spikeProgress = 0.0;
    game->man.isCentered = 1;

    game->statusState = STATUS_STATE_LIVES;

    init_status_lives(game);

    //time
    game->time  = 0;
    game->deathCountdown = 0;

    surface = IMG_Load("truespike2.png");
    if(surface == NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't loag image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->spikes = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("angryball.png");
    if(surface==NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't load image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->angryBall=SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("man0.png");
    if(surface==NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't load image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[0]=SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("man1.png");
    if(surface==NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't load image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[1]=SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("man2.png");
    if(surface==NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't load image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[2]=SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("man3.png");
    if(surface==NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't load image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->manFrames[3]=SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("spikedeath.png");
    if(surface==NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't load image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->deathFrames[SPIKE]=SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface=IMG_Load("brick.png");
    if(surface==NULL)
    {
        puts(SDL_GetError());
        printf("Couldn't load image\n\n");
        SDL_Quit();
        exit(1);
    }
    game->brick=SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    srand(time(NULL));
    int i;
    /*for(i=0;i<100;i++)
    {
        game->balls[i].x = rand()%640;
        game->balls[i].y = rand()%480;
    }*/

    //layout the ledges:
    game->ledges[0].x = 320 - 148;
    game->ledges[0].y = 240;
    for(i=0;i<100;i++)
    {
        game->ledges[i].w = 256.0;
        game->ledges[i].h = 64.0;
    }
    for(i=1;i<100;i++)
    {
        game->ledges[i].x = game->ledges[i-1].x + game->ledges[i-1].w + 30 + rand()%220;  //range of gap between consecutive gaps is 30<=Gap<=250
        game->ledges[i].y = 160 + rand()%240; //range of ledges y position from 160 to 400
    }
    //test:
    /*for(i=0;i<100;i++)
    {
        game->ledges[i].x = i*256;
        game->ledges[i].y = 240;
    }*/

}

void resetGame(GameState* game)
{
    game->man.isDead = 0;

    game->man.x = 320-8; //for centering : man.x = (window width/2) - (rectangle width/2)
    game->man.y = 240-30; //for centering : man.y = (window height/2) = (rectangle height/2)
    game->man.dx = 0;
    game->man.dy = 0;

    game->spike.x = 0;
    game->spike.y = 0;
    game->spike.w = 120;
    game->spike.h = 480;
    game->spike.dx = 2;

    //flags
    game->man.onLedge = 0;
    game->man.animFrame = 0;
    game->man.facingRight = 1;
    game->man.slowingdown = 0;
    game->scrollX = 0.0;
    game->spikeProgress = 0.0;
    game->man.isCentered = 1;

    game->statusState = STATUS_STATE_LIVES;

    init_status_lives(game);

    //time
    game->time  = 0;
    game->deathCountdown = 0;

    //layout the ledges:
    game->ledges[0].x = 320 - 148;
    game->ledges[0].y = 240;
    int i;
    for(i=0;i<100;i++)
    {
        game->ledges[i].w = 256.0;
        game->ledges[i].h = 64.0;
    }
    for(i=1;i<100;i++)
    {
        game->ledges[i].x = game->ledges[i-1].x + game->ledges[i-1].w + 30 + rand()%220;  //range of gap between consecutive gaps is 30<=Gap<=250
        game->ledges[i].y = 160 + rand()%240; //range of ledges y position from 160 to 400
    }
}

int main(int argc, char* argv[])
{
    //printf("Hello world!\n");
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    GameState gameState;

    SDL_Init(SDL_INIT_VIDEO);

    window=SDL_CreateWindow("Game Window",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            640,
                            480,
                            0
                            );
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    gameState.renderer = renderer;

    TTF_Init(); //initialize font system

    loadGame(&gameState);

    int done=0;

    while(!done)
    {

        done=processEvents(window, &gameState);

        process(&gameState);
        collisionProcess(&gameState);

        doRender(renderer, &gameState);

        //SDL_Delay(42);

    }

    SDL_DestroyTexture(gameState.angryBall);
    SDL_DestroyTexture(gameState.manFrames[0]);
    SDL_DestroyTexture(gameState.manFrames[1]);
    SDL_DestroyTexture(gameState.manFrames[2]);
    SDL_DestroyTexture(gameState.manFrames[3]);
    SDL_DestroyTexture(gameState.brick);
    if(gameState.label != NULL)
        SDL_DestroyTexture(gameState.label);
    TTF_CloseFont(gameState.font);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    TTF_Quit();

    SDL_Quit();

    /*SDL_SetRenderDrawColor(renderer,128,0,255,255);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer,255,255,255,255);

    SDL_Rect rect = {220, 140, 200, 200};
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);

    SDL_Delay(2000);

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    SDL_Quit();*/
    return 0;
}

