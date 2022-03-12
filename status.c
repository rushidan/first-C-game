#include <stdio.h>
#include <principal.h>

void init_status_lives(GameState *game)
{
    char str[32] = "";
    sprintf(str, "x %u", game->man.lives);

    SDL_Color white = {255, 255, 255, 255};

    SDL_Surface *tmp = TTF_RenderText_Blended(game->font, str, white);
    game->labelW = tmp->w; // taking the actual width of the text loaded in the surface
    game->labelH = tmp->h; //taking the actual height of the text loaded in the surface
    game->label = SDL_CreateTextureFromSurface(game->renderer, tmp);
    SDL_FreeSurface(tmp);
}

void draw_status_lives(GameState *game)
{
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);   //set renderer color to black
    SDL_RenderClear(game->renderer);    //draw the renderer black

    //draw the man
    SDL_Rect manRect = {320-105, 240-MAN_HEIGHT/2-5, MAN_WIDTH_STANDING, MAN_HEIGHT};
    SDL_RenderCopy(game->renderer,game->manFrames[0],NULL,&manRect);

    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255); //set renderer color to white

    SDL_Rect textRect = {320-45, 240-game->labelH/2, game->labelW, game->labelH};
    SDL_RenderCopy(game->renderer, game->label, NULL, &textRect);
}

void shutdown_status_lives(GameState *game)
{
    SDL_DestroyTexture(game->label);
    game->label = NULL;
}
