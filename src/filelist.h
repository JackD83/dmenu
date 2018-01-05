#ifndef _FILELIST_H_
#define _FILELIST_H_

#include "common.h"

int  filelist_init(char* title, char* executable, char* exec_path, char* select_path, int can_change_dirs);
void filelist_deinit();
int  filelist_draw(SDL_Surface* screen);
void filelist_animate(SDL_Surface* screen);
void filelist_osd(SDL_Surface* screen);
void loadPreview();
void setPreviewPath();

MenuState filelist_keypress(SDLKey key);
void filelist_keyup(SDLKey key);

#endif
