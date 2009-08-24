/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *                     sca <scahoo <at> gmail <dot> com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "menu.h"
#include "conf.h"
#include "filelist.h"
#include "persistent.h"
#include "sound.h"

extern cfg_t *cfg;
cfg_t *m;
cfg_t *mi;
cfg_t *smi;

enum eSE SEnum;

SDL_Surface* background;
SDL_Surface* cursor;

SDL_Surface** menu_icons;
SDL_Surface** menu_text;
int number_of_menu;

SDL_Surface*** menuitem_icons;
SDL_Surface*** menuitem_text;
int* number_of_menuitem;

SDL_Surface** submenuitem_icons;
SDL_Surface** submenuitem_text;
int number_of_submenuitem;

SDL_Surface* tmp_surface;

int current_menu_index;
int current_menuitem_index;
int current_submenuitem_index;

TTF_Font* menu_font;
TTF_Font* menuitem_font;

void submenu_open();
void submenu_close();

int menu_init()
{
    int i, j;
    SDL_Color color = {255,255,255,0};

    tmp_surface= IMG_Load(cfg_getstr(cfg, "Background"));
    if (!tmp_surface) {
        printf("Failed to load %s: %s\n", cfg_getstr(cfg, "Background"), IMG_GetError());
        return 1;
    }
    background = SDL_DisplayFormat(tmp_surface);
    SDL_FreeSurface(tmp_surface);

    /*fill_fb(background->pixels);*/

    tmp_surface = IMG_Load(cfg_getstr(cfg, "Cursor"));
    if (!tmp_surface) {
        printf("Failed to load %s: %s\n", cfg_getstr(cfg, "Cursor"), IMG_GetError());
        return 1;
    }
    cursor = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);

    // load font
    TTF_Init();
    menu_font = TTF_OpenFont(cfg_getstr(cfg, "Font"), 18);
    menuitem_font = TTF_OpenFont(cfg_getstr(cfg, "Font"), 14);

    // load menu
    number_of_menu = cfg_size(cfg, "Menu");
    menu_icons = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * number_of_menu);
    menu_text = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * number_of_menu);
    number_of_menuitem = (int*)malloc(sizeof(int) * number_of_menu);
    menuitem_icons = (SDL_Surface***)malloc(sizeof(SDL_Surface**) * number_of_menu);
    menuitem_text = (SDL_Surface***)malloc(sizeof(SDL_Surface**) * number_of_menu);

    for (i=0;i<number_of_menu;i++) {
        m = cfg_getnsec(cfg, "Menu", i);

        tmp_surface = IMG_Load(cfg_getstr(m, "Icon"));
        menu_icons[i] = SDL_DisplayFormatAlpha(tmp_surface);
        SDL_FreeSurface(tmp_surface);

        tmp_surface = TTF_RenderUTF8_Blended(menu_font, cfg_getstr(m, "Name"), color);
        menu_text[i] = SDL_DisplayFormatAlpha(tmp_surface);
        SDL_FreeSurface(tmp_surface);

        number_of_menuitem[i] = cfg_size(m, "MenuItem");
        menuitem_icons[i] = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * number_of_menuitem[i]);
        menuitem_text[i] = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * number_of_menuitem[i]);
        for (j=0;j<number_of_menuitem[i];j++) {
            mi = cfg_getnsec(m, "MenuItem", j);

            tmp_surface = IMG_Load(cfg_getstr(mi, "Icon"));
            if (!tmp_surface) {
                printf("Failed to load %s: %s\n", cfg_getstr(mi, "Icon"), IMG_GetError());
                return 1;
            }
            menuitem_icons[i][j] = SDL_DisplayFormatAlpha(tmp_surface);
            SDL_FreeSurface(tmp_surface);

            tmp_surface = TTF_RenderUTF8_Blended(menuitem_font, cfg_getstr(mi, "Name"), color);
            menuitem_text[i][j] = SDL_DisplayFormatAlpha(tmp_surface);
            SDL_FreeSurface(tmp_surface);
        }
    }

    // Restore menu position
    current_menu_index     = g_persistent->current_menu;
    current_menuitem_index = g_persistent->current_menuitem;

    if (current_menu_index < 0 || current_menu_index >= number_of_menu) {
        current_menu_index     = 0;
        current_menuitem_index = 0;
    } else if (current_menuitem_index < 0 || current_menuitem_index >= number_of_menuitem[current_menu_index]) {
        current_menuitem_index = 0;
    }

    // Init sound
    SE_Init();

    return 0;
}

void menu_deinit()
{
    int i, j;

    // Save current menu state
    g_persistent->current_menu     = current_menu_index;
    g_persistent->current_menuitem = current_menuitem_index;

    SDL_FreeSurface(background);
    SDL_FreeSurface(cursor);

    for (i=0;i<number_of_menu;i++) {
        SDL_FreeSurface(menu_icons[i]);
        SDL_FreeSurface(menu_text[i]);
        for (j=0;j<number_of_menuitem[i];j++) {
            SDL_FreeSurface(menuitem_icons[i][j]);
            SDL_FreeSurface(menuitem_text[i][j]);
        }
        free(menuitem_icons[i]);
        free(menuitem_text[i]);
    }

    free(menuitem_icons);
    free(menuitem_text);
    free(number_of_menuitem);
    free(menu_icons);
    free(menu_text);

    TTF_CloseFont(menu_font);
    TTF_CloseFont(menuitem_font);
    TTF_Quit();

    if (number_of_submenuitem > 0) submenu_close();

    // De-init sound
    SE_deInit();

}

void menu_draw(SDL_Surface* screen)
{
    int x, y;
    SDL_Rect dstrect, txtrect;
    int done;

    x = 0;
    y = 0;
    dstrect.x = 0;
    dstrect.y = 0;

    // clear screen
    //SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_BlitSurface(background, 0, screen, &dstrect);

    // draw menu
    dstrect.y = (screen->h - menu_icons[0]->h - 20) / 3; // assuming the font height for menu name is 20

    done = 0;
    int currently_drawing_menu = current_menu_index - 1;
    while (!done) {
        if (currently_drawing_menu >= 0) {
            SDL_BlitSurface(menu_icons[currently_drawing_menu], 0, screen, &dstrect);
            if (currently_drawing_menu == current_menu_index) { // used for drawing menu items
                x = dstrect.x;
                y = dstrect.y;
                txtrect.x = dstrect.x + menu_icons[currently_drawing_menu]->w / 2 -
                            menu_text[currently_drawing_menu]->w / 2;
                txtrect.y = dstrect.y + menu_icons[currently_drawing_menu]->h;
                SDL_BlitSurface(menu_text[currently_drawing_menu], 0, screen, &txtrect);
            }
            dstrect.x += menu_icons[currently_drawing_menu]->w;
        }
        else {
            dstrect.x += menu_icons[0]->w;
        }
        currently_drawing_menu++;
        if (dstrect.x >= screen->w) done = 1;
        if (currently_drawing_menu >= number_of_menu) done = 1;
        if (number_of_submenuitem > 0 && 
            currently_drawing_menu > current_menu_index) done = 1; // if submenu is open 
    }

    if (number_of_menuitem[current_menu_index] == 0) return; // exit if no menuitem for this menu

    // draw upper menu items
    int currently_drawing_menuitem = current_menuitem_index - 1;
    dstrect.x = x;

    // will not draw upper menu items if there is none, or we have opened a submenu
    if (current_menuitem_index > 0 && number_of_submenuitem == 0) {

        dstrect.y = y - menuitem_icons[current_menu_index][currently_drawing_menuitem]->h;
        done = 0;

        while (!done) {
            txtrect.x = dstrect.x + menuitem_icons[current_menu_index][currently_drawing_menuitem]->w;
            txtrect.y = dstrect.y +
                        (menuitem_icons[current_menu_index][currently_drawing_menuitem]->h -
                         menuitem_text[current_menu_index][currently_drawing_menuitem]->h) / 2;
            SDL_SetAlpha(menuitem_icons[current_menu_index][currently_drawing_menuitem], SDL_SRCALPHA, 128);
            SDL_BlitSurface(menuitem_icons[current_menu_index][currently_drawing_menuitem], 0, screen, &dstrect);
            SDL_SetAlpha(menuitem_text[current_menu_index][currently_drawing_menuitem], SDL_SRCALPHA, 128);
            SDL_BlitSurface(menuitem_text[current_menu_index][currently_drawing_menuitem], 0, screen, &txtrect);
            if (dstrect.y <= 0) done = 1;
            currently_drawing_menuitem--;
            if (currently_drawing_menuitem < 0) {
                done = 1;
            } else {
                dstrect.y -= menuitem_icons[current_menu_index][currently_drawing_menuitem]->h;
            }
        }
    }

    // draw lower menu items
    currently_drawing_menuitem = current_menuitem_index;
    dstrect.y = y + menu_icons[current_menu_index]->h + 20;
    done = 0;

    while (!done) {
        if (currently_drawing_menuitem == current_menuitem_index) {
            SDL_BlitSurface(cursor, 0, screen, &dstrect);
            SDL_SetAlpha(menuitem_icons[current_menu_index][currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
            SDL_SetAlpha(menuitem_text[current_menu_index][currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
        } else {
            SDL_SetAlpha(menuitem_icons[current_menu_index][currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);
            SDL_SetAlpha(menuitem_text[current_menu_index][currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);
        }

        SDL_BlitSurface(menuitem_icons[current_menu_index][currently_drawing_menuitem], 0, screen, &dstrect);
        txtrect.x = dstrect.x + menuitem_icons[current_menu_index][currently_drawing_menuitem]->w;
        if (number_of_submenuitem == 0) {
            txtrect.y = dstrect.y +
                        (menuitem_icons[current_menu_index][currently_drawing_menuitem]->h -
                         menuitem_text[current_menu_index][currently_drawing_menuitem]->h) / 2;
            SDL_BlitSurface(menuitem_text[current_menu_index][currently_drawing_menuitem], 0, screen, &txtrect);
        }
        dstrect.y += menuitem_icons[current_menu_index][currently_drawing_menuitem]->h;
        if (dstrect.y >= screen->h) done = 1;
        currently_drawing_menuitem++;
        if (currently_drawing_menuitem >= number_of_menuitem[current_menu_index]) done = 1;
        if (number_of_submenuitem > 0) done = 1; // we only draw the current menuitem if submenu is open
    }

    // draw sub menu if it's open
    // at this point, txtrect.x and txtrect.y should be at the position where we want to draw the current
    // submenuitem
    x = txtrect.x;
    y = txtrect.y;
    dstrect.x = x;
    if (number_of_submenuitem > 0) {
        // draw upper submenuitems
        currently_drawing_menuitem = current_submenuitem_index - 1;
        if (current_submenuitem_index > 0) {
            dstrect.y = y - submenuitem_icons[currently_drawing_menuitem]->h;
            done = 0;
            while (!done) {
                txtrect.x = dstrect.x + submenuitem_icons[currently_drawing_menuitem]->w;
                txtrect.y = dstrect.y +
                            (submenuitem_icons[currently_drawing_menuitem]->h -
                             submenuitem_text[currently_drawing_menuitem]->h) / 2;
                SDL_SetAlpha(submenuitem_icons[currently_drawing_menuitem], SDL_SRCALPHA, 128);
                SDL_BlitSurface(submenuitem_icons[currently_drawing_menuitem], 0, screen, &dstrect);
                SDL_SetAlpha(submenuitem_text[currently_drawing_menuitem], SDL_SRCALPHA, 128);
                SDL_BlitSurface(submenuitem_text[currently_drawing_menuitem], 0, screen, &txtrect);
                if (dstrect.y <= 0) done = 1;
                currently_drawing_menuitem--;
                if (currently_drawing_menuitem < 0) {
                    done = 1;
                } else {
                    dstrect.y -= submenuitem_icons[currently_drawing_menuitem]->h;
                }
            }
        }

        // draw lower submenuitems
        currently_drawing_menuitem = current_submenuitem_index;
        dstrect.y = y;
        done = 0;
        while (!done) {
            if (currently_drawing_menuitem == current_submenuitem_index) {
                SDL_BlitSurface(cursor, 0, screen, &dstrect);
                SDL_SetAlpha(submenuitem_icons[currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
                SDL_SetAlpha(submenuitem_text[currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
            } else {
                SDL_SetAlpha(submenuitem_icons[currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);
                SDL_SetAlpha(submenuitem_text[currently_drawing_menuitem], SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);
            }

            SDL_BlitSurface(submenuitem_icons[currently_drawing_menuitem], 0, screen, &dstrect);
            txtrect.x = dstrect.x + submenuitem_icons[currently_drawing_menuitem]->w;
            txtrect.y = dstrect.y +
                            (submenuitem_icons[currently_drawing_menuitem]->h -
                             submenuitem_text[currently_drawing_menuitem]->h) / 2;
            SDL_BlitSurface(submenuitem_text[currently_drawing_menuitem], 0, screen, &txtrect);

            dstrect.y += submenuitem_icons[currently_drawing_menuitem]->h;
            if (dstrect.y >= screen->h) done = 1;
            currently_drawing_menuitem++;
            if (currently_drawing_menuitem >= number_of_submenuitem) done = 1;
        }
    }

}

enum MenuState menu_keypress(SDLKey keysym)
{
    enum MenuState state = MAINMENU;

    if (keysym == SDLK_LEFT) {
        menu_previous();
    }
    else if (keysym== SDLK_RIGHT) {
        menu_next();
    }
    else if (keysym == SDLK_UP) {
        menuitem_previous();
    }
    else if (keysym == SDLK_DOWN) {
        menuitem_next();
    }
    else if (keysym == SDLK_LCTRL) {
        m = cfg_getnsec(cfg, "Menu", current_menu_index);
        mi = cfg_getnsec(m, "MenuItem", current_menuitem_index);
        if (number_of_submenuitem > 0) {
            mi = cfg_getnsec(mi, "SubMenuItem", current_submenuitem_index);
        }
        if (cfg_getbool(mi, "Selector")) {
            if (!filelist_init(cfg_getstr(mi, "Name"),
                               cfg_getstr(mi, "Executable"),
                               cfg_getstr(mi, "WorkDir"))) {
                state = FILELIST;
            } // else we are not able to initialise the filelist display
        } else {
            menuitem_run();
        }
    }
    else if (keysym == SDLK_LALT) {
        if (number_of_submenuitem > 0) submenu_close();
    }

    return state;
}

void menu_next()
{
    if (number_of_submenuitem > 0) {
        SEnum = OUT;
        SE_out( SEnum );
        return;
    } // need to close submenu before go to another menu
    current_menu_index++;
    SEnum = MENU_MOVE;
    SE_out( SEnum );
    if (current_menu_index == number_of_menu) current_menu_index = 0;
    current_menuitem_index = 0;
}

void menu_previous()
{
    if (number_of_submenuitem > 0) {
        SEnum = OUT;
        SE_out( SEnum );
        return;
    } // need to close submenu before go to another menu
    current_menu_index--;
    SEnum = MENU_MOVE;
    SE_out( SEnum );
    if (current_menu_index < 0) current_menu_index = number_of_menu - 1; 
    current_menuitem_index = 0;
}

void menuitem_next()
{
    if (number_of_submenuitem == 0) {
        current_menuitem_index++;
        if (current_menuitem_index == number_of_menuitem[current_menu_index]) current_menuitem_index = 0;
    }
    else {
        current_submenuitem_index++;
        if (current_submenuitem_index == number_of_submenuitem) current_submenuitem_index = 0;
    }
    SEnum = MENUITEM_MOVE;
    SE_out( SEnum );
}

void menuitem_previous()
{
    if (number_of_submenuitem == 0) {
        current_menuitem_index--;
        if (current_menuitem_index < 0) current_menuitem_index = number_of_menuitem[current_menu_index] - 1;
    }
    else {
        current_submenuitem_index--;
        if (current_submenuitem_index < 0) current_submenuitem_index = number_of_submenuitem - 1;
    }
    SEnum = MENUITEM_MOVE;
    SE_out( SEnum );
}

void menuitem_run()
{
    char* executable = cfg_getstr(mi, "Executable");
    if (executable)
        run_command(executable, NULL, cfg_getstr(mi, "WorkDir"));
    else
        submenu_open();
}

void submenu_open()
{
    int i;

    SEnum = DECIDE;
    SE_out( SEnum );

    SDL_Color color = {255,255,255,0};

    number_of_submenuitem = cfg_size(mi, "SubMenuItem");
    submenuitem_icons = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * number_of_submenuitem);
    submenuitem_text = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * number_of_submenuitem);
    
    for (i=0;i<number_of_submenuitem;i++) {
        smi = cfg_getnsec(mi, "SubMenuItem", i);

        tmp_surface = IMG_Load(cfg_getstr(smi, "Icon"));
        if (!tmp_surface) {
            printf("Failed to load %s: %s\n", cfg_getstr(smi, "Icon"), IMG_GetError());
        } else {
            submenuitem_icons[i] = SDL_DisplayFormatAlpha(tmp_surface);
            SDL_FreeSurface(tmp_surface);
        }

        tmp_surface = TTF_RenderUTF8_Blended(menuitem_font, cfg_getstr(smi, "Name"), color);
        submenuitem_text[i] = SDL_DisplayFormatAlpha(tmp_surface);
        SDL_FreeSurface(tmp_surface);
    }    

    current_submenuitem_index = 0;
}

void submenu_close()
{
    int i;
    SEnum = CANCEL;
    SE_out( SEnum );
    for (i=0;i<number_of_submenuitem;i++) {
        SDL_FreeSurface(submenuitem_icons[i]);
        SDL_FreeSurface(submenuitem_text[i]);
    }

    free(submenuitem_icons);
    free(submenuitem_text);
    
    number_of_submenuitem = 0;
}
