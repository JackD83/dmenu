/*
 *  Copyright (C) 2009 sca <scahoo <at> gmail <dot> com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#ifndef __DOSD_H__
#define __DOSD_H__

#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>

int dosd_init();
void dosd_deinit();
void dosd_show(SDL_Surface *surface);
void dosd_color_reset();

bool dosd_is_locked();
bool dosd_is_charging();

typedef enum {
    BAT_LEVEL_BEST=3900,
    BAT_LEVEL_FAIR=3700,
    BAT_LEVEL_LOW=3611
} BatteryLevel;

typedef enum {
    BAT_STATE_EMPTY = 0,
    BAT_STATE_LEVEL1,
    BAT_STATE_LEVEL2,
    BAT_STATE_FULL,
	BAT_STATE_CHARGING,
    BAT_STATE_MAX
} BatteryState;

#endif//__DOSD_H__
