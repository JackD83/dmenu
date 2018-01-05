/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>, 
                       Timothy Soehnlin <timothy.soehnlin@gmail.com>
 *
 *  Authors: <mr.rookie1@gmail.com>,
 *           <timothy.soehnlin@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <png.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "env.h"
#include "filelist.h"
#include "conf.h"
#include "basec.h"
#include "main.h"
#include "menu.h"

#define MAX_IMAGE_JOBS 40
int CURRENT_JOB = 0;
extern cfg_t *menu_active_item_config;
pthread_t image_exporter[MAX_IMAGE_JOBS];

void run_internal_command(char* command, char* args, char* workdir);

void clear_last_command()
{    
    struct stat st;
    if (stat(DMENU_COMMAND_FILE, &st) == 0) {
        remove(DMENU_COMMAND_FILE);
    }
}

int filesystem_writeable()
{
    int ret;
    FILE* f = fopen(DMENU_CONF_TEMP, "w");
    ret = (f != NULL);
    if (f) fclose(f);
    return ret;
}

/* this is a simplified version when init is used to spawn dmenu */
void run_command(char* executable, char* args, char* workdir)
{   
    log_debug("Running Command: %s, %s, %s", executable, args, workdir);
    
    if (internal_command(executable)) {
        run_internal_command(executable, args, workdir);
        return;
    }
    
    char** args_list = build_arg_list(executable, args);
    //Make local copy of work dir
    
    char tmp_work[PATH_MAX]; strcpy(tmp_work, "");
    if (workdir != NULL) strcpy(tmp_work, workdir);
    
    if (can_write_fs()) {
        // launch the program
        execute_next_command(tmp_work, args_list);
    } else {
        execute_inline_command(tmp_work, args_list);
    }
}

void run_internal_command(char* command, char* args, char* workdir)
{
    if (!args) return;

    switch (get_command(command)) 
    {
        case THEMESELECT:      conf_themeselect(args);      break;
        case COLORSELECT:      conf_colorselect(args);      break;
        case BACKGROUNDSELECT: conf_backgroundselect(args); break;
    }
}

char** build_arg_list(char* commandline, char* args) 
{
    // build the args list for exec()
    char** args_list = new_str_arr(0);
    char* token;
    const char delimeter[] = " ";
    int len = 0;

      log_message("Building cmd for '%s' ", commandline);
    
    //if commandline file exists, dont break up spaces
    if (file_exist (commandline)) {
        append_str(args_list, len, strdup(commandline));
    } else {
        //not a file, still buggy but works for filelist files
        while ((token = strsep(&commandline, delimeter))) {
            if (token[0] != '\0') {
                append_str(args_list, len, strdup(token));
            }
        }
    }
        
    // filelist.c will pass the selected filename in args. filename
    // may contain spaces.
    if (args) {
        append_str(args_list, len, strdup(args));
    }
    append_str(args_list, len, NULL);
        
    return args_list;
}

void execute_inline_command(char* dir, char** args)
{
    deinit(SHUTDOWN);
    SDL_Quit();

    int i = 0;
    if (dir!=NULL && strlen(dir) > 0)
    {
        change_dir(dir);
    }
    
    // launch the program and it should not return,
    // otherwise it means we are not able to execute the application

    execvp(args[0], args);
    
    log_message("Unable to execute command - ");
    for (i=0;*(args+i);i++) printf("\"%s\" ", *(args+i)); printf("\n");
    // it should not return, otherwise it means we are not able to execute the application
    free_str_arr(args);
    _exit(1);
}

void execute_next_command(char* dir, char** args) 
{   
    //Write next command to commandfile
    FILE* out = load_file(DMENU_COMMAND_FILE, "w");

    char* name  = cfg_getstr(menu_active_item_config, "Name");
    int center = cfg_getbool(menu_active_item_config, "Center");
    int zoom = cfg_getbool(menu_active_item_config, "Zoom");
    int volumeControl = cfg_getbool(menu_active_item_config, "VolumeControl");
    

    log_message("Executing %s, center %d", name, center);
    
    if (out != NULL) {

        fprintf(out, "#!/bin/sh\n");

        if(center) {
            fprintf(out, "echo 1 > /proc/jz/lcd_a320\n");
        } else if(zoom) {
             fprintf(out, "echo 2 > /proc/jz/lcd_a320\n");
        }

        if(volumeControl) {
              fprintf(out, "/mnt/mmc/dmenu/volume/setVolume &\n");
        }
        
        //Write change dir
        if (dir!=NULL && strlen(dir) > 0)
        {
            fprintf(out, "cd \"%s\"\n", dir);
        }
        
        //Write command
        int i = 0;
        for (i=0;*(args+i);i++) 
        {
            fprintf(out, "\"%s\" ", *(args+i)); 
        }    
        fprintf(out, "\n");
        if(center || zoom) {          
            fprintf(out, "echo 0 > /proc/jz/lcd_a320\n");
        }

        if(volumeControl) {
            fprintf(out, "killall setVolume\n");
        }

        fclose(out);
        
    } else {
        log_error("Unable to write next command");
    }
    
    free_str_arr(args);
    
    //Exit program, and let shell script call DMENU_COMMAND_FILE
    quit();
}

#define IF_CMD_THEN(c, k) if (strcmp(c, COMMAND_##k)==0) return k;
InternalCommand get_command(char* cmd) {
    IF_CMD_THEN(cmd, THEMESELECT);
    IF_CMD_THEN(cmd, BACKGROUNDSELECT);
    IF_CMD_THEN(cmd, COLORSELECT);
    return 0;
}

char* relative_file(char* root, char* file) 
{    
    char* out = new_str(PATH_MAX); out[0] = '\0';
    if (file[0] != '/') strcat(out, root); //If it really is relative
    strcat(out, file);
    return out;
}

int change_dir(char* path) 
{
    int rc = chdir(path);
    if (rc) {
        log_error("Unable to change to directory %s", path);
    }
    return rc;
}

char* get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

char* removeExtension(char* mystr) {
   
    char *retstr;
    char *lastdot;
    if (mystr == NULL)
         return NULL;
    if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
        return NULL;
    strcpy (retstr, mystr);
    lastdot = strrchr (retstr, '.');
    if (lastdot != NULL)
        *lastdot = '\0';
    return retstr;
}

int checkExtension(char* extension, char* list) { 
    if(strcmp(list, "%ALL%") == 0) {
        return 1;
    }

    char tmp_list[strlen(list)]; strcpy(tmp_list, list);  

    char *token;
    token = strtok(tmp_list, " ");
    while(token != NULL)
    {
        if(strcmp(token, extension) == 0) {        
            return 1;
        }       
        token = strtok(NULL," ");
    }
    return 0;
}

// Determines if entry references the parent dir as ".."
int is_back_dir(const struct dirent *dr) 
{
    return !strcmp(dr->d_name, "..");
}

int alphasort_i(const struct dirent **a, const struct dirent **b)
{
    return(stricmp((*a)->d_name, (*b)->d_name));
}


char* read_first_line( char* file ) 
{
    FILE* fp = load_file( file , "r" );
    char* out = new_str(100);
    int i = fscanf(fp, "%s", out);
    fclose(fp);
    return (i == 0 || i == EOF) ? NULL : out;
}

FILE* load_file_and_handle_fail(char* file, char* mode, int die_on_fail) 
{
    log_debug("Opening file: %s", file);
    
    FILE* out = fopen(file, mode);    
    if (out == NULL) {
        log_error("Failed to open %s", file);
        if (die_on_fail) exit(EXIT_FAILURE);
    }
    return out;
}

FILE* load_file_or_die(char* file, char* mode) 
{
    return load_file_and_handle_fail(file,mode,1);
}

FILE* load_file(char* file, char* mode) 
{
    return load_file_and_handle_fail(file,mode,0);
}

SDL_Surface* load_image_file_with_format( char* file , int alpha, int fail_on_notfound ) 
{
    SDL_Surface* out = NULL, *tmp_surface;

    tmp_surface = IMG_Load(file);
    if (tmp_surface == NULL) {
        if (fail_on_notfound) {
            log_error("Failed to load %s: %s", file, IMG_GetError());
            SDL_FreeSurface(out);
            return out;
        } else {
            return NULL;
        }
    }
    
    if ( alpha ) {
        out = SDL_DisplayFormatAlpha(tmp_surface);
    } else {
        out = SDL_DisplayFormat(tmp_surface);
    } 
    
    SDL_FreeSurface(tmp_surface);
    return out;
}

SDL_Surface* load_image_file( char* file ) {
    return load_image_file_with_format(file, 1, 1);
}

SDL_Surface* load_image_file_no_alpha( char* file ) {
    return load_image_file_with_format(file, 0, 1);
}

SDL_Surface* render_text(char* text, TTF_Font* font, SDL_Color* color, int solid) {
    SDL_Surface* out = NULL, *tmp_surface;
    if (solid == 0) {
        tmp_surface = TTF_RenderUTF8_Blended(font, text, *color);
    } else {
        tmp_surface = TTF_RenderUTF8_Solid(font, text, *color);
    }
    out = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);
    return out;
}

SDL_Surface* draw_text(char* text, TTF_Font* font, SDL_Color* color) {
    return render_text(text, font, color, 0);
}

SDL_Color* parse_color_string(char* str) {
    SDL_Color* out = new_item(SDL_Color);
    SDL_Color color = {255,255,255,0};
    int r = 0, g = 0, b = 0;
    char* format;

    switch (str[0]) {
        case '#':
            format = "#%2x%2x%2x"; // #RRGGBB
            break;
        case '(':
            format = "(%3d,%3d,%3d)"; // (R,G,B)
            break;
        default:
            format = "%3d,%3d,%3d"; // R,G,B
    }
    
    sscanf(str, format, &r, &g, &b);
    color.r = (Uint8)r;
    color.g = (Uint8)g;
    color.b = (Uint8)b;
    memcpy(out, &color, sizeof(SDL_Color));
    return out;
}

SDL_Color* load_color_file(char* filename) {
    char *color = read_first_line(filename);
    SDL_Color* out = parse_color_string(color);
    free_erase(color);
    return out;
}
 
void init_rect_pos(SDL_Rect* rect, int x, int y) {
    init_rect(rect, x, y, -1, -1);
}

void  init_rect(SDL_Rect* rect, int x, int y, int w, int h) {
    rect->x = x;
    rect->y = y;
    if (w>=0) rect->w = w;
    if (h>=0) rect->h = h;
}

SDL_Surface* create_surface(int w, int h, int depth, int color, int alpha)
{
    SDL_Surface *tmp;    
    Uint32 rm=0xFF, gm=0xFF, bm=0xFF, am=0xFF;
    int r=0xFF&(color>>16), g=0xFF&(color>>8), b=0xFF&(color);
    
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
    on the endianness (byte order) of the machine */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rm<<=24; gm<<=16; bm<<=8; 
    #else
    gm<<=8; bm<<=16; am<<=24;
    #endif
    
    tmp =  SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, depth, rm, gm, bm, (alpha < 0 || depth <= 24) ? 0 : am);
    
    if (color>=0) {
        if (alpha >= 0)  {
            SDL_FillRect(tmp, 0, SDL_MapRGBA(tmp->format, r,g,b,alpha));
        } else {
            SDL_FillRect(tmp, 0, SDL_MapRGB(tmp->format, r,g,b));
        }
    }
    return tmp;
}

/**
 *  Alpha blends a 32bit Surface using per pixel alpha blending. It will 
 *  take any given surface and scale each pixel's alpha value down 
 *  by alpha. If alpha is a power of 2 it should scale much faster than any
 *  other values as it can use bitwise shifting instead of multiplication. The
 *  following alpha values are the special values.
 *     255 ->  Does nothing (alpha blending by 100% is just ignored)
 *     127, 63, 31, 15, 7, 3, 1 -> Quick divide using bitwise shifting
 *     0 -> no math, just clears out alpha channel
 *     Everything else -> alpha/255 * pixel alpha
 */
void alphaBlendSurface( SDL_Surface* s, int alpha ) 
{
            
    if (alpha == 255) return;

    
    #ifdef DINGOO_BUILD

    Uint32 *pixels = (Uint32*)s->pixels, *p;

    float alpha_ratio;
    const register int pitch = s->pitch;
    register int off = 0;
    register int x = 0;
    int max_off = s->h*pitch,
        max_x= pitch,
        msk = s->format->Amask,
        alpha_step = 8;

    #define map_pixel(op)\
        while (off<max_off) {\
            while (x<max_x) {\
                p = (Uint32*)((int)pixels + x);\
                *p = op; x+=4; }\
                off += pitch;max_x = off+pitch;}
    
    switch (alpha) {
        case 127: --alpha_step;
        case 63:  --alpha_step;
        case 31:  --alpha_step;
        case 15:  --alpha_step;
        case 7:   --alpha_step;
        case 3:   --alpha_step;
        case 1:   --alpha_step;
            map_pixel((*p & ~msk) | (*p>>alpha_step & msk));
            break;
        case 0:
            map_pixel((*p & ~msk));
            break;
        default:
            alpha_ratio = alpha/255.0f;
            #if SDL_BYTEORDER == SDL_BIG_ENDIAN
                map_pixel((*p & ~msk) | ((int)(*p * alpha_ratio) & 0xFF));
            #else
                map_pixel((*p & ~msk) | (int)((*p>>24) * alpha_ratio) << 24);
            #endif
            break;
    }
    #undef map_pixel
    
    #endif
}

/** 
 * Blits Alpha Blended Surface (scales opacity by alpha/255)
 */
void blitSurfaceAlpha ( SDL_Surface* src, SDL_Rect* src_rect, SDL_Surface* dst, SDL_Rect* dst_rect, int alpha)
{
    if (alpha == 0) return;
    
    SDL_Surface* tmp = copy_surface(src);
    alphaBlendSurface(tmp, alpha);
    SDL_BlitSurface(tmp,  src_rect, dst, dst_rect);
    SDL_FreeSurface(tmp);
}

/**
 * Optimized for 16bit images.  Will not work any any others
 */
SDL_Surface* shrink_surface(SDL_Surface *src, int width, int height)
{
    if (!src) return NULL;
    
    int h = src->h, w = src->w;
    
    if(width>w || height>h || h <=0 || w <= 0) return NULL;
    
    float fpx = (w*1.0f)/(width*1.0f), fpy =(h*1.0f)/(height*1.0f);
    int y,x;
    
    SDL_Surface *dst = SDL_CreateRGBSurface(
        src->flags, width, height, 16,
        src->format->Rmask, src->format->Gmask, 
        src->format->Bmask, src->format->Amask);

    register int src_pitch = src->pitch, dst_pitch = dst->pitch;
    register int src_y_off = 0, dst_y_off = 0, dst_x_off=0;
    Uint16 *src_pixels = (Uint16*)src->pixels, *dst_pixels = (Uint16*)dst->pixels;
    int max_w = 2*width;
    
    y = height;
    while (y--) {
        
        dst_y_off = dst_pitch*y;
        src_y_off = src_pitch*(int)(y*fpy);
        dst_x_off = max_w;
        
        x= width;
        while (x--) {
            dst_x_off -= 2;
            *(Uint16*)((int)dst_pixels + dst_y_off+dst_x_off) = 
                *(Uint16*)((int)src_pixels + src_y_off+(int)(x*fpx)*2);
        }
    }
    
    return dst;
}

SDL_Surface* tint_surface(SDL_Surface* src, int color, int alpha) 
{    
    SDL_Surface *out;
    Uint32 rm=0xFF&(color>>16), gm=0xFF&(color>>8), bm=0xFF&(color), am=0xFF&alpha;
    
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
    on the endianness (byte order) of the machine */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rm<<=24; gm<<=16; bm<<=8; 
    #else
    gm<<=8; bm<<=16; am<<=24;
    #endif
    
    out =  SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, rm, gm, bm, alpha?am:0);
    SDL_SetAlpha(src, 0, src->format->alpha);
    SDL_SetAlpha(out, SDL_SRCALPHA, out->format->alpha);
    SDL_BlitSurface(src, NULL, out, NULL);
    SDL_SetAlpha(src, SDL_SRCALPHA, src->format->alpha);
    return out;
}

SDL_Surface* copy_surface(SDL_Surface* src)
{
    Uint32 rm=0xFF, gm=0xFF, bm=0xFF, am=0xFF;
    
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
    on the endianness (byte order) of the machine */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rm<<=24; gm<<=16; bm<<=8; 
    #else
    gm<<=8; bm<<=16; am<<=24;
    #endif
    
    SDL_Surface *out =  SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, rm, gm, bm, am);
    SDL_SetAlpha(src, 0, src->format->alpha);
    SDL_SetAlpha(out, SDL_SRCALPHA, out->format->alpha);
    SDL_BlitSurface(src, NULL, out, NULL);
    SDL_SetAlpha(src, SDL_SRCALPHA, src->format->alpha);
    return out;
}

/**
 * Copied from mailing list post, cleaned up a little bit. Works Now!
 */
int export_surface_as_png(char *filename, SDL_Surface *surface)
{        
    /* Creating the output surface to save */
    SDL_Surface* surf = create_surface(surface->w, surface->h, 32, 0,0);
    SDL_BlitSurface(surface, NULL, surf, NULL);
    
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    int i=0, colortype, rc = -1;
    png_bytep *rows = NULL;
    
    // Get resources
    if (!(fp = fopen(filename, "wb"))) goto done;
    if (!(png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL))) goto done;
    if (!(info_ptr = png_create_info_struct(png_ptr))) goto done;
    if (setjmp(png_jmpbuf(png_ptr))) goto done;

    colortype = PNG_COLOR_MASK_COLOR
        | (surf->format->palette ? PNG_COLOR_MASK_PALETTE : 0)
        | (surf->format->Amask   ? PNG_COLOR_MASK_ALPHA   : 0);

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, surf->w, surf->h, 8, colortype, 
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    /* Writing the image */
    png_write_info(png_ptr, info_ptr);
    png_set_packing(png_ptr);
    
    rows = (png_bytep*) malloc(sizeof(png_bytep)*surf->h);
    for (;i<surf->h;i++) rows[i] = (png_bytep)(Uint8 *)surf->pixels + i*surf->pitch;
    png_write_image(png_ptr, rows);
    png_write_end(png_ptr, info_ptr);
    rc = 0;
    
done: //Cleanup
    if (png_ptr)
        png_destroy_write_struct(&png_ptr, &info_ptr);
    if (fp) {
        #ifdef FSYNC
            int fno = fileno(fp);
            fsync(fno);
        #endif
        fclose(fp);
    }
    free_erase(rows);
    free_surface(surf);
    return rc;
}


int export_surface_as_bmp(char *filename, SDL_Surface *surface) 
{
    return SDL_SaveBMP(surface, filename);
}

void run_export_job(ImageExportJob* job)
{
    if (job->surface) //If surface is still around
    {
        export_surface_as_png(job->file, job->surface); 
        
        //Copy over mtime/atime as the dingoo doesn't have a clock and so 
        // these values default to 0, and we need to be able to check mtimes
        // to know if we need to recache
        struct utimbuf new_time = {job->orig_stat->st_atime, job->orig_stat->st_mtime}; 
        utime(job->file, &new_time);
    }
    
    //free_surface(job->surface); 
    free_erase(job->file); 
    free_erase(job->orig_stat); 
    free_erase(job);
}

void init_export_job(char* old_filename, char* new_filename, SDL_Surface* sfc) 
{
    //Using BMP for now, b/c it is faster, should move to png but it may
    // need to be spawned in it's own thread
    ImageExportJob* job = new_item(ImageExportJob); {
        
        struct stat* tmp_stat = new_item(struct stat);
        stat(old_filename, tmp_stat);
        job->orig_stat = tmp_stat;            
        job->file = strdup(new_filename); 
        job->surface = sfc;
    }
    
    CURRENT_JOB = wrap(CURRENT_JOB+1,0,MAX_IMAGE_JOBS-1);
    pthread_create(&image_exporter[CURRENT_JOB], NULL, (void*)run_export_job, (void*)job); 
}

SDL_Surface* resize_image(SDL_Surface* in, int width, int height)
{
    SDL_Surface *tmp = shrink_surface(in, width, height);
    free_surface(in);
    return tmp;
}

int file_exist (char *filename)
{
  struct stat buffer;   
  return (stat (filename, &buffer) == 0);
}

SDL_Surface* load_resized_image(char* file, int width, int height)
{

    if (cant_write_fs())
    { 
        return resize_image(load_image_file_with_format(file,0,0), width, height);
    }
    
    struct stat orig_stat, new_stat, dir_stat;
    
    char new_file[PATH_MAX], new_dir[PATH_MAX];
    
    //Copy path of file
    int end = strrpos(file, '/');
    strncpy(new_dir, file, end);
    new_dir[end] = '\0';
    strcat(new_dir, THUMBNAILS_PATH);
    
    sprintf(new_file, "%s/%s_%03dx%03d", new_dir, strrchr(file, '/')+1, width, height);
    
    SDL_Surface *out = load_image_file_with_format(new_file, 0, 0), *tmp;
    
    if (out == NULL) { //If not created
        out = load_image_file_no_alpha(file);
        tmp = shrink_surface(out, width, height);
        free_surface(out);
        out = tmp;
        
        if(stat(new_dir,&dir_stat) != 0) {
            int rc = mkdir(new_dir, 0777);
            if (rc != 0) {
                log_error("Unable to make dir: (%d) %s", rc, new_dir);
                return tmp;
            }   
        }
        init_export_job(file, new_file, out);
    } else {
        stat(file, &orig_stat);
        stat(new_file, &new_stat);
    
        //If image timestamp is different
        if (orig_stat.st_mtime != new_stat.st_mtime) 
        {
            //Clear cache and reload image
            remove(new_file); 
            free_surface(out);
            out = load_resized_image(file, width, height);
        }
    }
    
    return out;
}
