/*
 *  Copyright (C) 2009 sca <scahoo <at> gmail <dot> com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "dosd.h"
#include "images.h"
#include "env.h"
#include "../common.h"
#include "resource.h"

extern int image_pallette[3];
extern cfg_t *cfg_main;
bool show_speed = false;
SDL_Rect speed_dst_rect1, speed_dst_rect2;
TTF_Font* speed_font;
SDL_Color* dosd_color;

// ___ Globals ___________________________________________
// Global state
struct {
    FILE* proc_battery;
    FILE* proc_gpio3;
    FILE* proc_gpio1;
	FILE* proc_cgm;
    FILE* proc_usb;
	int mhz;  
    BatteryState battery;
    unsigned int update_counter;
    // Measured in SDL_GetTick()s
    uint32_t next_update;
} g_state;

// Processed images
image_t g_images[IMG_MAX];

// ___ Functions ___________________________________________________
void _blit(SDL_Surface *surface, image_e which_image, ...);
void _update();

// Color should be passed in RRGGBB format, e.g. 0xFFFFFF for white
int dosd_init()
{
    log_debug("Initializing");
    int i;

    memset(&g_state, 0, sizeof(g_state));
    memset(&g_images, 0, sizeof(g_images));
    
#ifdef DINGOO_BUILD
    g_state.proc_battery = fopen(BATTERY_DEVICE, "rb");
    if (!g_state.proc_battery) goto init_error;
    
	g_state.proc_cgm = fopen(CPU_DEVICE, "r");
	if (!g_state.proc_cgm) goto init_error;

    g_state.proc_usb = fopen(USB_DEVICE, "r");
	if (!g_state.proc_usb) goto init_error;

	show_speed = cfg_getbool(cfg_main, "SpeedDisp");

    init_rect_pos( &speed_dst_rect1, CPU_DISP_X, CPU_DISP_Y);
    init_rect_pos( &speed_dst_rect2, CPU_DISP_X+1, CPU_DISP_Y);
    speed_font = get_theme_font(CPU_FONT_SIZE);
    
    g_state.mhz = 0;    
	
#endif
    
    dosd_color = get_theme_font_color();
    
    for (i = 0; i < IMG_MAX; i++)
    {
        const image_data_t* image_data;
        image_t* image;
        uint32_t* buf;
        SDL_Surface* surface;
        int j;
        
        image_data =  c_images[i];
        image      = &g_images[i];
        
        image->w  = image_data->w;
        image->h  = image_data->h;
        
        buf = malloc(image->w * image->h * 4);
        if (!buf) goto init_error;
        
        for (j = 0; j < image->w * image->h; j++)
        {
            buf[j] = image_pallette[image_data->data[j]];
        }
        
		int color = (dosd_color->r) << 24;
		color += (dosd_color->g) << 16;
		color += (dosd_color->b) << 8;

		surface = SDL_CreateRGBSurfaceFrom(buf, image->w, image->h, 32, image->w * 4,
			(0xff<<24) & color,
			(0xff<<16) & color,
			(0xff<<8) & color,
			0xff);
        
        if (!surface) goto init_error;
        
        // Convert surface to target surface format
        image->sfc = SDL_DisplayFormatAlpha(surface);
        if (!image->sfc) 
        {
            SDL_FreeSurface(surface);
            goto init_error;
        }
        
        SDL_FreeSurface(surface);
        free(buf);
    }
    
    // Init g_state
    _update();
    
    return 1;
init_error:
    dosd_deinit();
    return 0;
}

void dosd_color_reset() {
	dosd_color = get_theme_font_color();
}

void dosd_deinit()
{
    log_debug("De-initializing");
    int i;
    
#ifdef DINGOO_BUILD
	g_state.mhz=0;

    if (g_state.proc_battery)
        fclose(g_state.proc_battery);
     
	if (g_state.proc_cgm)
		fclose(g_state.proc_cgm);

    if (g_state.proc_usb)
		fclose(g_state.proc_usb);
#endif
    
    for (i = 0; i < IMG_MAX; i++)
    {
        if (g_images[i].sfc)
            SDL_FreeSurface(g_images[i].sfc);
    }
}

void dosd_show(SDL_Surface* surface)
{
    image_e battery_status;
    
    if (SDL_GetTicks() >= g_state.next_update)
    {
        _update();
    }
    
    // Battery
    _blit(surface, IMG_BATTERY, -1);
    
    // This is a bit of a hack, as it relies on battery_state_e
    // To have the same index as image_e
    battery_status = g_state.battery;
	if (false ) battery_status = BAT_STATE_CHARGING;
    if (battery_status != BAT_STATE_EMPTY)
    {
        _blit(surface, battery_status, -1);
    }
    
	if (show_speed) {
		SDL_Surface* speed_display;
		char speed_text[10] = " ";
		int disp_mhz = g_state.mhz;
		sprintf(speed_text, "%03d MHz", disp_mhz);
		speed_display = draw_text(speed_text, speed_font, dosd_color);
	    SDL_BlitSurface(speed_display, 0, surface, &speed_dst_rect1 );
	    SDL_BlitSurface(speed_display, 0, surface, &speed_dst_rect2 );
		free_surface(speed_display);
	}
}

inline bool dosd_is_locked() {
    return false;
}

void _blit(SDL_Surface *surface, image_e which_image, ...) {
    /*
        image_e are expected in the variable arguments.
        The image will be offset by the combined width + padding
        of the images in the varargs.
        ALWAYS pass -1 as the last argument!
    */
    SDL_Rect dst;
    va_list ap;
    image_e  padding;
    image_t* image;
    
    if (which_image < 0 || which_image >= IMG_MAX)
        return;
    
    image = &g_images[which_image];
    
    dst.y = DOSD_PADDING;
    dst.x = surface->w - image->w - DOSD_PADDING; 
    
    va_start(ap, which_image);
    while ((padding = va_arg(ap, image_e)) != -1)
    {
        dst.x -= DOSD_PADDING + g_images[padding].w;
    }
    
    va_end(ap);
    
    SDL_BlitSurface(image->sfc, NULL, surface, &dst);
}

void _update()
{
#ifdef DINGOO_BUILD
    char buf[128];
    int mvolts;
    uint32_t gpio;
    
    rewind(g_state.proc_battery);
    fgets(buf, 127, g_state.proc_battery);
    sscanf(buf, "%d", &mvolts);
   
    // Battery charge level
    if      (mvolts >= BAT_LEVEL_BEST) g_state.battery = BAT_STATE_FULL;
    else if (mvolts >= BAT_LEVEL_FAIR) g_state.battery = BAT_STATE_LEVEL2;
    else if (mvolts >= BAT_LEVEL_LOW ) g_state.battery = BAT_STATE_LEVEL1;
    else                                  g_state.battery = BAT_STATE_EMPTY;

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
	rewind(g_state.proc_usb);
    read = getline(&line, &len, g_state.proc_usb);
    
    if(read == 4) { //equals usb      
         g_state.battery = BAT_STATE_CHARGING;
    }     

	// get mhz
	if ( show_speed ) {
		rewind(g_state.proc_cgm);
		char line[80];
		while( fgets(line, 80, g_state.proc_cgm) != NULL) {
			if ( strstr(line, "PLL Freq") ) {
				g_state.mhz = atoi(strstr(strstr(line, ": "), " "));
			}
		}
	}
#else  
    g_state.battery     = BAT_STATE_EMPTY;
	g_state.mhz			= 0;
#endif
    
    // Next update
    g_state.next_update = SDL_GetTicks() + DOSD_UPDATE_INTERVAL;
    g_state.update_counter++;
}

bool dosd_is_charging() {
	return false;
}
