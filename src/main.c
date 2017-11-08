/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *                     Timothy Soehnlin <timothy.soehnlin@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <SDL.h>
#include <SDL_image.h>
#include "env.h"
#include "dingoo.h"
#include "common.h"
#include "resource.h"

#include "main.h"
#include "menu.h"
#include "filelist.h"
#include "colorpicker.h"
#include "imageviewer.h"

#include "loading.h"
#include "conf.h"
#include "brightness.h"
#include "volume.h"
#include "watch.h"
#include "sound.h"
#include "persistent.h"
#include "dosd/dosd.h"

extern cfg_t *cfg_main;
extern int conf_changed;

static Uint32 next_time;
SDL_Surface *screen, *screen_cache;
MenuState state = MAINMENU;
int program_done;
bool escape_held;

Uint32 time_left(void)
{
    Uint32 now = SDL_GetTicks();
    return next_time <= now ? 0 : next_time - now;
}

int init_system()
{

    //Move to dmenu root dir
    change_dir(DMENU_PATH);

    // load config
    if (conf_load(NULL))
        return 1;

    if (can_write_fs())
        clear_last_command();

    // Read saved persistent state
    if (!persistent_init())
    {
        log_error("Unable to initialize persistent memory");
    }

    //Init OSD
    {
        brightness_init();
        volume_init();
        watch_init();
        if (!dosd_init())
        {
            log_error("Unable to initialize OSD");
            return 1;
        }
    }

    // Init sound
    sound_init();
    // init menu
    if (menu_init())
    {
        log_error("Unable to load menu");
        return 1;
    }

    return 0;
}

int init_display()
{

    // initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        log_error("Unable to init SDL: %s", SDL_GetError());
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // disable mouse pointer
    SDL_ShowCursor(SDL_DISABLE);

    // create a new window
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH, SDL_SWSURFACE);
    if (!screen)
    {
        log_error("Unable to set %dx%d video: %s", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        return 1;
    }
    else
    {
        loading_init(screen);
    }

    screen_cache = create_surface(SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0xFFFFFF, 0);

    //Ready fonts
    TTF_Init();
    return 0;
}

int init()
{
    log_debug("Initializing");

    set_write_fs(filesystem_writeable());

#ifdef DINGOO_BUILD
    // Need to ignore SIGHUP if we are started by init.
    // In FB_CloseKeyboard() of SDL_fbevents.c , it does
    // ioctl(tty0_fd, TIOCNOTTY, 0) to detach the process from tty,
    // this will send a SIGHUP to us and dmenu will quit at
    // SDL_Init() below if it's not ignored.
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
    {
        log_error("Unable to ignore SIGHUP");
    }
#endif

    int rc = init_display() || init_system();
    next_time = SDL_GetTicks() + TICK_INTERVAL;
    state = MAINMENU;
    return rc;
}

void deinit(DeinitLevel level)
{
    log_debug("De-initializing, level %d", level);

    int shutdown = level == SHUTDOWN;

    // de-init everything
    colorpicker_deinit();
    filelist_deinit();
    imageviewer_deinit();
    state = MAINMENU;

    if (shutdown)
    {
        sound_deinit();
        brightness_deinit();
        volume_deinit();
        dosd_deinit();
    }

    menu_deinit();

    if (level == SHUTDOWN)
    {
        //Close down fonts
        TTF_Quit();
        conf_unload();
        loading_deinit();
    }
}

void reload(DeinitLevel level)
{
    deinit(level);
    if (level == RELOAD_THEME)
    {
        conf_reload_theme(THEME_NAME);
    }
    menu_init();
}

void draw_osd(SDL_Surface *screen)
{
    dosd_show(screen);
    if (volume_enabled())
        volume_show(screen);
    if (brightness_enabled())
        brightness_show(screen);
    if (watch_enabled())
        watch_show(screen);
}

/**
 * Draw Screen
 */
void update_display()
{

    // DRAWING STARTS HERE
    int recache = 0;
    switch (state)
    {
    case MAINMENU:
        recache = menu_draw(screen);
        break;
    case FILELIST:
        recache = filelist_draw(screen);
        break;
    case IMAGEVIEWER:
        recache = imageviewer_draw(screen);
        break;
    case COLORPICKER:
        recache = colorpicker_draw(screen);
        break;
    }

    if (!recache)
    {
        SDL_BlitSurface(screen_cache, NULL, screen, NULL);
    }
    else
    {
        SDL_BlitSurface(screen, NULL, screen_cache, NULL);
    }

    switch (state)
    {
    case MAINMENU:
        menu_animate(screen);
        break;
    case FILELIST:
        filelist_animate(screen);
        break;
    case IMAGEVIEWER:
        imageviewer_animate(screen);
        break;
    case COLORPICKER:
        colorpicker_animate(screen);
        break;
    }

    switch (state)
    {
    case MAINMENU:
        menu_osd(screen);
        draw_osd(screen);
        break;
    case FILELIST:
        filelist_osd(screen);
        break;
    case IMAGEVIEWER:
        imageviewer_osd(screen);
        break;
    case COLORPICKER:
        colorpicker_osd(screen);
        break;
    }

    // finally, update the screen :)
    SDL_Flip(screen);

    // wait for remaining time of the frame
    SDL_Delay(time_left());
    next_time += TICK_INTERVAL;
}

void handle_global_key(SDLKey key)
{
    //Handle OSD activity
    Direction dir = getKeyDir(key);
    if (state == MAINMENU)
    {
        if (escape_held)
        {
            log_debug("No functions defined yet for esc + key");
        }
        else
        {
            switch (key)
            {
            case DINGOO_BUTTON_VOL_DOWN:
            case DINGOO_BUTTON_VOL_UP:
                conf_changed = 1;
                volume_change(dir);
                sound_out(GLOBAL_KEY);
                break;
            case DINGOO_BUTTON_X:
            case DINGOO_BUTTON_Y:
                conf_changed = 1;
                brightness_change(dir);
                sound_out(GLOBAL_KEY);
                break;
            default:
                break;
            }
        }
    } 
}

   /* Print all information about a key event */
   void PrintKeyInfo(SDL_KeyboardEvent * key)
   {
       /* Is it a release or a press? */
       if (key->type == SDL_KEYUP)
           printf("Release:- ");
       else
           printf("Press:- ");

       /* Print the hardware scancode first */
       printf("Scancode: 0x%02X", key->keysym.scancode);
       /* Print the name of the key */
       printf(", Name: %s", SDL_GetKeyName(key->keysym.sym));
       /* We want to print the unicode info, but we need to make */
       /* sure its a press event first (remember, release events */
       /* don't have unicode info                                */
       if (key->type == SDL_KEYDOWN)
       {
           /* If the Unicode value is less than 0x80 then the    */
           /* unicode value can be used to get a printable       */
           /* representation of the key, using (char)unicode.    */
           printf(", Unicode: ");
           if (key->keysym.unicode < 0x80 && key->keysym.unicode > 0)
           {
               printf("%c (0x%04X)", (char)key->keysym.unicode,
                      key->keysym.unicode);
           }
           else
           {
               printf("? (0x%04X)", key->keysym.unicode);
           }
       }
       printf("\n");
       /* Print modifier info */
       PrintModifiers(key->keysym.mod);
   }

   /* Print modifier info */
   void PrintModifiers(SDLMod mod)
   {
       printf("Modifers: ");

       /* If there are none then say so and return */
       if (mod == KMOD_NONE)
       {
           printf("None\n");
           return;
       }

       /* Check for the presence of each SDLMod value */
       /* This looks messy, but there really isn't    */
       /* a clearer way.                              */
       if (mod & KMOD_NUM)
           printf("NUMLOCK ");
       if (mod & KMOD_CAPS)
           printf("CAPSLOCK ");
       if (mod & KMOD_LCTRL)
           printf("LCTRL ");
       if (mod & KMOD_RCTRL)
           printf("RCTRL ");
       if (mod & KMOD_RSHIFT)
           printf("RSHIFT ");
       if (mod & KMOD_LSHIFT)
           printf("LSHIFT ");
       if (mod & KMOD_RALT)
           printf("RALT ");
       if (mod & KMOD_LALT)
           printf("LALT ");
       if (mod & KMOD_CTRL)
           printf("CTRL ");
       if (mod & KMOD_SHIFT)
           printf("SHIFT ");
       if (mod & KMOD_ALT)
           printf("ALT ");
       printf("\n");
   }

void quit()
{
    program_done = 1;
}

void listen()
{

    // program main loop
    program_done = 0;
    SDLKey key;
    SDL_Event event;
    MenuState prevstate;
    escape_held = false;

    int last_key_time = SDL_GetTicks();
    int inactive_delay = cfg_getint(cfg_main, "DimmerDelay") * 1000;
    int shutdown_delay = cfg_getint(cfg_main, "ShutdownDelay") * 1000;

    //Allow for easier menu nav
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    while (!program_done)
    {
        // message processing loop
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
            // exit if the window is closed
            case SDL_QUIT:
                quit();
                break;

            case SDL_KEYUP:              
                key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE)
                    escape_held = false;
                break;

            // check for keypresses
            case SDL_KEYDOWN:
                //PrintKeyInfo(&event.key);
                key = event.key.keysym.sym;

                if (key == SDLK_ESCAPE)
                {
                    escape_held = true;
                    break;
                }
                if ((key == SDLK_RETURN) && (escape_held))
                {
                    quit();
                    break;
                }

                if (dosd_is_locked())
                    break;

                last_key_time = SDL_GetTicks();
                if (brightness_is_dimmed())
                {
                    brightness_dim(0);
                    break;
                }

                prevstate = state;
                switch (state)
                {
                case MAINMENU:
                    state = menu_keypress(key);
                    break;
                case FILELIST:
                    state = filelist_keypress(key);
                    break;
                case IMAGEVIEWER:
                    state = imageviewer_keypress(key);
                    break;
                case COLORPICKER:
                    state = colorpicker_keypress(key);
                    break;
                }
                if (state == MAINMENU && prevstate != state)
                {
                    menu_force_redraw(screen_cache);
                }

                brightness_dim(0);
                handle_global_key(key);
                break;
            } // end switch
        }     // end of message processing

        if ((inactive_delay > 0 && inactive_delay < (SDL_GetTicks() - last_key_time)) && !(brightness_is_dimmed()))
        {
            brightness_dim(1);
        }
        if (shutdown_delay > 0 && shutdown_delay < (SDL_GetTicks() - last_key_time))
        {
            if (!(dosd_is_charging()))
            {
                log_error("No activity for %d seconds, powering off.", (shutdown_delay / 1000));
                run_command("poweroff", "", "/");
            }
            else
            {
                // reset the shutdown clock so it doesn't power off as soon as the USB
                // is unplugged
                last_key_time = SDL_GetTicks() + inactive_delay;
            }
        }

        update_display(screen, state);
    } // end main loop
}

int main(int argc, char **argv)
{
    int rc = init();
    log_debug("Fully initialized");

    if (rc == 0)
    {
        listen(); //main loop
        deinit(SHUTDOWN);

        // all is well ;)
        log_debug("Exited cleanly");
    }

    return rc;
}
