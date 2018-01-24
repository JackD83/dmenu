/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "env.h"
#include "conf.h"
#include "filelist.h"
#include "resource.h"
#include "menu.h"

#include "sound.h"
#include "dingoo.h"

#define ends_with_slash(str) (str[strlen(str)-1] == '/')

//Get access to the current menu item
extern cfg_t* menu_active_item_config;
extern cfg_t* cfg_main;
extern cfg_t *cfg;
extern SDL_Surface* background;

SDL_Surface* list_bg;
SDL_Surface* list_sel;
SDL_Surface* list_title;
SDL_Surface* preview;

SDL_Surface* list_dir_icon;
SDL_Surface* list_file_icon;

TTF_Font* list_font;
SDL_Color* list_font_color;
char * preview_path = "";
char * previewBasePath = NULL;
int showPreview = 1;
int hideExtension;

typedef struct FileListGlobal 
{
    int status_changed;
    
    struct dirent **namelist;
    struct stat *statlist;
    SDL_Surface* list_filename[FILES_PER_PAGE];
    int num_of_files; // total number of files under current directory
    int current_list_start; // index of the file at top of current page
    int current_highlight; // index of the file being highlighted
    int at_root; //Whether or not the listing is at the root of the file system
    int can_change_dir;
    int is_initted;
    
    char current_path[PATH_MAX];
    char work_path[PATH_MAX];
    char current_executable[PATH_MAX];
    
} FileListGlobal;

FileListGlobal fl_global;


SDL_Surface* filelist_render_text(char* text) 
{
    return draw_text(text, list_font, list_font_color);
}

void filelist_fill()
{  
    int i, j;
    //Clear old entries
    for (i=0;i<FILES_PER_PAGE;i++) 
    {
        if (fl_global.list_filename[i]) 
        {
            SDL_FreeSurface(fl_global.list_filename[i]);
            fl_global.list_filename[i] = NULL;
        }
    }
    
    //Write new entries
    for (i=0,j=fl_global.current_list_start; i<FILES_PER_PAGE; i++,j++) 
    {
        if (j < fl_global.num_of_files) 
        {
            if(hideExtension) {
                char* name = removeExtension(fl_global.namelist[j]->d_name);
                fl_global.list_filename[i] = filelist_render_text(name);
                free(name);
            } else {
                fl_global.list_filename[i] = filelist_render_text(fl_global.namelist[j]->d_name);
            }           
        }
    }
}

int filelist_filter(const struct dirent *dptr)
{       
    char* extList  = cfg_getstr(menu_active_item_config, "Extensions");
    return (fl_global.can_change_dir && !fl_global.at_root && is_back_dir(dptr)) 
            || (dptr->d_name[0] != '.' && (checkExtension(get_filename_ext(dptr->d_name), extList ) || dptr->d_type == DT_DIR) );
}

int filelist_sort(const struct dirent **a, const struct dirent **b) 
{
    //Note this fails for symlinks of dirs, but because the dingux runs on a Fat32 drive, this
    // shouldn't be an issue.
    if ((*a)->d_name[0] == '.' || (*b)->d_name[0] == '.') 
    {
        return is_back_dir(*a) ? -1 : 1;
    }

    int c = (*a)->d_type==DT_DIR, d=(*b)->d_type==DT_DIR;
    return c==d ? alphasort_i(a,b) : c ? -1 : 1;
}

int filelist_get_list(char* path)
{
    int i = 0;

    fl_global.at_root = !strcmp(path,"/");
    fl_global.num_of_files = scandir(path, &fl_global.namelist, filelist_filter, (void*)filelist_sort);
    fl_global.current_list_start = fl_global.current_highlight = 0;

    if (fl_global.num_of_files < 0) 
    {
        fl_global.num_of_files = 0;
        return -1;
    }
    else {
        fl_global.statlist = new_array(struct stat, fl_global.num_of_files);
        for (i=0;i<fl_global.num_of_files;i++) 
        {
            char *filename = new_str(strlen(path) + strlen(fl_global.namelist[i]->d_name) + 2);
            strcpy(filename, path);
            strcat(filename, "/");
            strcat(filename, fl_global.namelist[i]->d_name);
            stat(filename, &fl_global.statlist[i]);
            free_erase(filename);
        }

        strcpy(fl_global.current_path, path);
        filelist_fill();
        return 0;
    }
}

void filelist_clear_list()
{
    free_erase(fl_global.statlist);
    
    if (fl_global.namelist) 
    {
        int i, j;
        for (i=0;i<fl_global.num_of_files;i++) free_erase(fl_global.namelist[i]);
        
        free_erase(fl_global.namelist);
    
        for (i=0, j=fl_global.current_list_start;
            (i < FILES_PER_PAGE) && (j < fl_global.num_of_files);
            i++, j++) 
        {
            free_surface(fl_global.list_filename[i]);
        }
    }
    
    fl_global.current_list_start = fl_global.current_highlight = fl_global.num_of_files = 0;
}

int filelist_init(char* title, char* executable, char *exec_path, char* select_path, int can_change_dirs)
{
    log_debug("Initializing");
    
    //Make sure it is something
    if (executable == 0) executable = "";
    
    // load font
    list_font       = get_theme_font(18);
    list_font_color = get_theme_font_color();

    //Setup UI
    list_bg        = load_theme_image(cfg_getstr(cfg, "ListBackground"));
    list_sel       = load_theme_image(cfg_getstr(cfg, "ListSelector"));
    list_dir_icon  = load_theme_image(cfg_getstr(cfg, "ListDirIcon"));
    list_file_icon = load_theme_image(cfg_getstr(cfg, "ListFileIcon"));
    list_title     = filelist_render_text(title);
    previewBasePath    =  cfg_getstr(menu_active_item_config, "Previews");
    hideExtension = cfg_getbool(cfg_main, "HideExtensions");  

    free_surface(preview);
  
    fl_global.status_changed = 1;

    // Prep path/executable vars, determine filelist_theme status
    strcpy(fl_global.current_executable, executable);
    strcpy(fl_global.work_path, exec_path);

    fl_global.can_change_dir = can_change_dirs;
    
    // read files
    char real_path[PATH_MAX];
    if (!realpath(select_path, real_path)) 
    {
        log_error("Failed to get real path of directory %s", select_path);
        strcpy(real_path, "/"); //Default to root when real_path fails
        //return 1;
    }
    if (filelist_get_list(real_path)) 
    {
        log_error("Failed to read directory %s", real_path);
        return 1;
    }

    fl_global.is_initted = 1;
    
    return 0;
}

void filelist_deinit()
{
    if (!fl_global.is_initted) return;
    log_debug("De-initializing");
    
    fl_global.is_initted = 0;

    free_surface(list_bg);
    free_surface(list_sel);
    free_surface(list_title);
    free_surface(list_dir_icon);
    free_surface(list_file_icon);
    free_surface(preview);
    free_font(list_font);
    free_color(list_font_color);

    filelist_clear_list();
}

int filelist_draw(SDL_Surface* screen)
{
    int i;
    SDL_Rect image_rect, text_rect, preview_rect;

    if (!fl_global.status_changed) return 0;
    init_rect_pos(&image_rect, 0,0);
    init_rect_pos(&text_rect, 0,0);

    init_rect_pos(&preview_rect, PREVIEW_OFFSET_RIGHT ,30);

    // clear screen
    SDL_BlitSurface(background, 0, screen, &image_rect);
    SDL_BlitSurface(list_bg,    0, screen, &image_rect);



    text_rect.y = FILE_LIST_OFFSET;
    for (i=0; i<FILES_PER_PAGE; i++) 
    {
        if (fl_global.list_filename[i]) 
        {
            if (i == fl_global.current_highlight) 
            {
                SDL_BlitSurface(list_sel, 0, screen, &text_rect);
            }
            
            if (S_ISDIR(fl_global.statlist[fl_global.current_list_start+i].st_mode)) 
            {
                SDL_BlitSurface(list_dir_icon, 0, screen, &text_rect);
                text_rect.x += list_dir_icon->w;
            }
            else {
                SDL_BlitSurface(list_file_icon, 0, screen, &text_rect);
                text_rect.x += list_file_icon->w;
            }
            
            SDL_BlitSurface(fl_global.list_filename[i], 0, screen, &text_rect);
            text_rect.x = 0;
            text_rect.y += FILE_ENTRY_HEIGHT;
        }
    }

    SDL_BlitSurface(preview,  0, screen, &preview_rect);

    fl_global.status_changed = 0;
    return 1;
}

void filelist_animate(SDL_Surface* screen) {}
void filelist_osd(SDL_Surface* screen) 
{
    SDL_Rect rect;
    init_rect_pos(&rect, 0,0);
    SDL_BlitSurface(list_title, 0, screen, &rect);
}

void filelist_shift_page(Direction dir) 
{
    int i, 
        start = 0, 
        end = FILES_PER_PAGE-1, 
        delta = 1;
   

    if (dir == PREV) 
    {
        start = FILES_PER_PAGE-1;
        end = 0;
        delta = -1;
    }
        
    SDL_FreeSurface(fl_global.list_filename[start]);
    for (i=start;i!=end;i+=delta) 
    {
        fl_global.list_filename[i] = fl_global.list_filename[i+delta];
    }
    
    fl_global.current_list_start += delta;

    if(hideExtension) {
        char* name = removeExtension(fl_global.namelist[fl_global.current_list_start+end]->d_name);
        fl_global.list_filename[end] = filelist_render_text(name);
        free(name);
    } else {
            fl_global.list_filename[end] = filelist_render_text(fl_global.namelist[fl_global.current_list_start+end]->d_name);
    }   
}

void filelist_wrap_page(Direction dir) 
{
    int size = FILES_PER_PAGE;

    if (dir != PREV) 
    { //start at top
        fl_global.current_list_start = 0; 
        fl_global.current_highlight  = 0;
    } 
    else {
        fl_global.current_list_start = max(fl_global.num_of_files - size, 0);
        fl_global.current_highlight  = min(fl_global.num_of_files - 1, size - 1); 
    }
    filelist_fill();
}

void filelist_move_entry(Direction dir) 
{
    int delta    = (dir == PREV) ? -1 : 1;
    int size     = FILES_PER_PAGE;
    int max      = fl_global.num_of_files;
    int next_pos = fl_global.current_highlight + delta;
    int next_abs_pos = fl_global.current_list_start + next_pos; 
    
    sound_out( MENUITEM_MOVE );
    
    if (in_bounds(next_pos,0,size) && next_pos < max) //If moving within page
    {
        fl_global.current_highlight += (PREV==dir ? -1 : 1);
    } 
    else if (in_bounds(next_abs_pos, 0, max)) //Slide page
    {
        filelist_shift_page(dir);
    }
    else  //Wrap Around
    {
        filelist_wrap_page(dir);
    }    

    setpreviewBasePath();
    
    fl_global.status_changed = 1;   
}


void filelist_move_page(Direction dir)
{
    int size = FILES_PER_PAGE, delta = (dir==PREV?-size:size);;
    int start = fl_global.current_list_start;
    int next = start + delta;
    sound_out( MENU_MOVE );
    fl_global.current_list_start =  bound(next, 0, max(fl_global.num_of_files-size, 0));
    fl_global.current_highlight = 0;
    
    filelist_fill();

    setpreviewBasePath();
    
    fl_global.status_changed = 1;
}

void filelist_move_left()
{
    sound_out( CANCEL );

    //reset preview
    preview_path = "";  
    loadPreview();

   log_debug("move up");

    if (fl_global.at_root) 
    {
        return;
    } 
    else {
        char* last_separator = strrchr(fl_global.current_path, '/');
        if (!last_separator)
        {
            log_error("Unable to find path separator - %s", fl_global.current_path);
            return;
        } 
        else if (last_separator == fl_global.current_path) {
            fl_global.current_path[1] = '\0';
        } 
        else {
            *last_separator = '\0';
        }
        filelist_clear_list();
        filelist_get_list(fl_global.current_path);
        fl_global.status_changed = 1;
    }


}

void filelist_move_right()
{
    char temp_path[PATH_MAX];
    int i = fl_global.current_list_start+fl_global.current_highlight;
    
    sound_out( DECIDE );
    
    if (S_ISDIR(fl_global.statlist[i].st_mode) && !is_back_dir(fl_global.namelist[i])) 
    {
        strcpy(temp_path, fl_global.current_path);
        if (!ends_with_slash(fl_global.current_path)) 
        {
            strcat(temp_path, "/");
        }
        strcat(temp_path, fl_global.namelist[i]->d_name);
        filelist_clear_list();
        if (filelist_get_list(temp_path) != 0)
        {
            filelist_get_list(fl_global.current_path);
        }
        fl_global.status_changed = 1;
    }
} 

void filelist_changedir(Direction dir) 
{
    if (fl_global.can_change_dir) 
    {
        dir == PREV ? filelist_move_left() :filelist_move_right();
    }
}

void filelist_store_dir()
{
    if (fl_global.can_change_dir && can_write_fs()) 
    {
        conf_dirselect(menu_active_item_config, fl_global.current_path);
    }
}

void togglePreview() {
    if(showPreview) {
        showPreview = 0;
    } else {
        showPreview = 1;
    }
    loadPreview();
     fl_global.status_changed = 1;
}

void setpreviewBasePath() {
    if(previewBasePath == NULL) {
        return;
    }

    int i = fl_global.current_list_start+fl_global.current_highlight;
    if ( fl_global.namelist[i]->d_type != DT_DIR) //d_type may not work on every file system. Does on Ext and FAT -> works for Dingoox
    {   
        char file_name[PATH_MAX] = "";
        char* name = removeExtension(fl_global.namelist[i]->d_name);


        strcat(file_name, previewBasePath);
        if (!ends_with_slash(previewBasePath)) 
        {
            strcat(file_name, "/");
        }
        strcat(file_name, name);
        strcat(file_name, ".png");
        free(name);

        preview_path = file_name;             
    } else {
         preview_path = "";  
            log_debug("path empty"); 
    } 
}

void loadPreview() {  
    if(previewBasePath == NULL){
        return;
    }
    
    free_surface(preview);

    if(!showPreview) {
        return;
    }

    if(file_exist (preview_path)) {
        preview = load_image_file(preview_path);    
    }   

    fl_global.status_changed = 1;
     
   
}

// Since filelist_run() has multiple purpose, the next state
// need to be returned from this function.
MenuState filelist_run()
{
    char file_name[PATH_MAX];
    int i = fl_global.current_list_start+fl_global.current_highlight;

    if (fl_global.can_change_dir && S_ISDIR(fl_global.statlist[i].st_mode)) 
    {
        filelist_changedir(is_back_dir(fl_global.namelist[i])?PREV:NEXT);
        return FILELIST;
    }
    
    strcpy(file_name, fl_global.current_path);
    if (!ends_with_slash(fl_global.current_path)) 
    {
        strcat(file_name, "/");
    }
    strcat(file_name, fl_global.namelist[i]->d_name);
    
    // If this is theme selection, we will return here from run_command();
    // If it's running a program, it will not return.
    if (fl_global.current_executable[0] != '\0') 
    {
        run_command(fl_global.current_executable, file_name, fl_global.work_path);
    }
    else {
        run_command(file_name, NULL, fl_global.current_path);
    }
    return MAINMENU;
}

MenuState filelist_keypress(SDLKey key)
{
    Direction dir = getKeyDir(key);
    
    switch (key) 
    {
        case DINGOO_BUTTON_L:
        case DINGOO_BUTTON_R:
            filelist_move_page(dir);
            break;
        case DINGOO_BUTTON_UP:
        case DINGOO_BUTTON_DOWN:
            filelist_move_entry(dir);
            break;
        case DINGOO_BUTTON_LEFT:
        case DINGOO_BUTTON_RIGHT:
            filelist_changedir(dir);
            break;
        case DINGOO_BUTTON_START:
            filelist_store_dir();
            break;
        case DINGOO_BUTTON_B:
            filelist_deinit();
            return MAINMENU;
        case DINGOO_BUTTON_A:
            return filelist_run();
        case DINGOO_BUTTON_Y:
            togglePreview();
            break;
        default:break;
    }

    return FILELIST;
}

void filelist_keyup(SDLKey key) {     
     switch (key) 
    {
        case DINGOO_BUTTON_L:
        case DINGOO_BUTTON_R:       
        case DINGOO_BUTTON_UP:
        case DINGOO_BUTTON_DOWN:
           loadPreview();
            break;
        default:break;
    }
}