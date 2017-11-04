#ifndef __MAIN_H__
#define __MAIN_H__

#include "common.h"

void PrintKeyInfo( SDL_KeyboardEvent *key );
void PrintModifiers( SDLMod mod );
void deinit(DeinitLevel level);
void reload(DeinitLevel level);
void quick_quit();
void quit();
void update_display();
void draw_osd(SDL_Surface* screen);


#endif