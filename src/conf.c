/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include "resource.h"
#include "common.h"
#include "conf.h"
#include "watch.h"
#include "dosd/dosd.h"
#include "main.h"
#include "menu.h"

#define is_set(f, x) (((f) & (x)) == (f))
cfg_opt_t submenuitem_opts[] = {
    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_STR("Executable", 0, CFGF_NONE),
    CFG_STR("WorkDir", ".", CFGF_NONE),
    CFG_BOOL("Selector", cfg_false, CFGF_NONE),   
    CFG_STR("SelectorDir", 0, CFGF_NONE),
    CFG_BOOL("Center", cfg_false, CFGF_NONE),
    CFG_BOOL("Zoom", cfg_false, CFGF_NONE),
    CFG_BOOL("VolumeControl", cfg_false, CFGF_NONE),
    CFG_STR("Extensions", "%ALL%", CFGF_NONE),
     CFG_STR("Previews",NULL, CFGF_NONE),
    CFG_END()
};

cfg_opt_t menuitem_opts[] = {
    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_STR("Executable", 0, CFGF_NONE),
    CFG_STR("WorkDir", ".", CFGF_NONE),
    CFG_BOOL("Selector", cfg_false, CFGF_NONE),
    CFG_STR("SelectorDir", 0, CFGF_NONE),
    CFG_BOOL("Center", cfg_false, CFGF_NONE),
    CFG_BOOL("Zoom", cfg_false, CFGF_NONE),
    CFG_BOOL("VolumeControl", cfg_false, CFGF_NONE),
    CFG_STR("Extensions", "%ALL%", CFGF_NONE),
    CFG_STR("Previews",NULL, CFGF_NONE),
    CFG_SEC("SubMenuItem", submenuitem_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_FUNC("include", cfg_include),
    CFG_END()
};

cfg_opt_t menu_opts[] = {

    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_SEC("MenuItem", menuitem_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_FUNC("include", cfg_include),
    CFG_END()
};

cfg_opt_t opts[] = {
    CFG_STR("Background", 0, CFGF_NONE),
    CFG_STR("Cursor", 0, CFGF_NONE),
    CFG_STR("Font", 0, CFGF_NONE),
    CFG_STR("FontColor", 0, CFGF_NONE),
    CFG_INT("ItemTextPadLeft", 0, CFGF_NONE),

    CFG_BOOL("VolDisp", cfg_false, CFGF_NONE),
    CFG_BOOL("BrightDisp", cfg_false, CFGF_NONE),

    CFG_STR("ListBackground", 0, CFGF_NONE),
    CFG_STR("ListSelector", 0, CFGF_NONE),
    CFG_STR("ListDirIcon", 0, CFGF_NONE),
    CFG_STR("ListFileIcon", 0, CFGF_NONE),
    CFG_STR("LoadingScreen", 0, CFGF_NONE),

    CFG_STR("MenuSE", 0, CFGF_NONE),
    CFG_STR("MenuItemSE", 0, CFGF_NONE),
    CFG_STR("DecideSE", 0, CFGF_NONE),
    CFG_STR("CancelSE", 0, CFGF_NONE),
    CFG_STR("OutSE", 0, CFGF_NONE),
    CFG_STR("TestSE", 0, CFGF_NONE),

    CFG_SEC("Menu", menu_opts, CFGF_MULTI | CFGF_TITLE),

    CFG_FUNC("include", cfg_include),

    CFG_END()
};

cfg_opt_t standalone_menu_opts[] = {
    CFG_SEC("MenuItem", menuitem_opts, CFGF_TITLE),
    CFG_END()
};

cfg_opt_t standalone_opts[] = {
    CFG_SEC("Menu", standalone_menu_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_END()
};


cfg_opt_t selectordir_opts[] = {
    CFG_STR_LIST("Key", 0, CFGF_NONE),
    CFG_STR("Dir", 0, CFGF_NONE),
    CFG_END()
};

cfg_opt_t main_opts[] = {
    CFG_STR("Theme", "default", CFGF_NONE),
    CFG_STR_LIST("SearchPath", "{}", CFGF_NONE),
    //@decprecated
    CFG_BOOL("AllowDynamicThemeChange", cfg_false, CFGF_NONE), 
    CFG_BOOL("VolDisp", cfg_false, CFGF_NONE),
    CFG_BOOL("BrightDisp", cfg_false, CFGF_NONE),
    CFG_BOOL("WatchDisp", cfg_false, CFGF_NONE),
	CFG_BOOL("SpeedDisp", cfg_false, CFGF_NONE),
    CFG_BOOL("ReadOnly", cfg_false, CFGF_NONE),
    CFG_BOOL("HideExtensions", cfg_true, CFGF_NONE),
    CFG_INT("SndVol", 50, CFGF_NONE),
    CFG_INT("Bright", 99, CFGF_NONE),
    CFG_STR("Background", 0, CFGF_NONE),
    CFG_INT("DimmerDelay", DIMMER_DELAY, CFGF_NONE),
	CFG_INT("ShutdownDelay", SHUTDOWN_DELAY, CFGF_NONE),
    CFG_STR("FontColor", 0, CFGF_NONE),
    CFG_STR("Font", 0, CFGF_NONE),
    
    CFG_SEC("SelectorDir", selectordir_opts, CFGF_MULTI),
    CFG_END()
};

cfg_t *cfg;
cfg_t *cfg_main;
int conf_changed = 0;

char* theme_path;

cfg_opt_t* conf_dupopts(cfg_opt_t* opts);
int path_filter(const struct dirent *dptr);

cfg_t* conf_from_file(cfg_opt_t* opts, char* file)
{
    log_debug("Opening config file: %s", file);
    cfg_t* out = cfg_init(opts, 0);
    int rc = cfg_parse(out, file);
    if (rc != CFG_SUCCESS) {
        char* action = rc == CFG_FILE_ERROR ? "load" : "parse";
		if(strstr(action,"parse")) {
	        log_debug( "Unable to %s config file:%s . rc = %d", action, file, rc);
		} else {
	        log_error( "Unable to %s config file:%s . rc = %d", action, file, rc);
	        out = NULL;
		}
    }
    return out;
}

void move_file(char* from, char* to) {

    struct stat st; 

    log_debug("Moving %s to %s", from, to);
    
    //If file saved and has size
    if (stat(from, &st)==0 && st.st_size > 20) {
        rename(from, to);
    } else {
        log_error("Writing configuration file failed: %s", to);
    }
}

FILE* open_conf_file(cfg_t* cfg, char* file) {
    
    log_debug("Opening conf file for writing: %s", file);
        
    FILE *fp;
    fp = load_file(file, "w");
    if (fp == NULL) return 0;
    
    cfg_print(cfg, fp);
    return fp;
}

void close_conf_file(FILE* fp) {
    
    log_debug("Closing conf file");
    #ifdef FSYNC
        int file_no = fileno(fp);
        fsync(file_no);
    #endif
    fclose(fp);
}

int conf_to_file(cfg_t* cfg, char* file) {
    
    char* tmp;

    tmp = new_str(strlen(file)+5); {
        strcpy(tmp, file);
        strcat(tmp, ".tmp");
    }
    
    FILE* fp = open_conf_file(cfg, tmp);
    if (fp == NULL) return 0;
    close_conf_file(fp);
    move_file(tmp, file);    
    free_erase(tmp);

    return 1;
}

char* THEME_CONF_FILE;

int conf_load_theme(char* theme) 
{
    int rc = 0;
    
    // build theme_dir
    char theme_dir[PATH_MAX]; theme_dir[0] = '\0';
    
    if (theme[0] != '/')  //If relative
    {
        strcat(theme_dir, DMENU_THEMES);
    }
    
    strcat(theme_dir, theme);
    
    rc = change_dir(theme_dir);
    if (rc) return rc;

    //Build theme path
    strcpy(THEME_PATH, theme_dir);
    strcpy(THEME_NAME, strrchr(THEME_PATH, '/')+1);
    strcat(THEME_PATH, "/");
    
    
    //Build theme conf file
    THEME_CONF_FILE = theme_file("theme.cfg");

    // load theme.cfg
    change_dir(THEME_PATH);    
    cfg = conf_from_file(opts, THEME_CONF_FILE);
    if (cfg == NULL) return CFG_PARSE_ERROR;
    change_dir(DMENU_PATH);
    
    return 0;
}

void conf_process_theme()
{
    struct dirent **namelist;
    int num_of_files, i, j;
    cfg_t* tmp;
    
    // load dmenu.cfg files from SearchPath
    char search_path[PATH_MAX];
    char work_path[PATH_MAX];
    for (j=0;j<cfg_size(cfg_main, "SearchPath");j++) {
        char* search_path_in_conf = cfg_getnstr(cfg_main, "SearchPath", j);
        if (search_path_in_conf[0] == '/') { // SearchPath is absolute
            strcpy(search_path, search_path_in_conf);
        } else { // SearchPath is relative to dmenu directory
            char * tmp = dmenu_file(search_path_in_conf);
            strcpy(search_path, tmp);
            free_erase(tmp);
        }
        
        num_of_files = scandir(search_path, &namelist, path_filter, alphasort);
        for (i=0;i<num_of_files;i++) {
            strcpy(work_path, search_path);
            strcat(work_path, "/");
            strcat(work_path, namelist[i]->d_name);
            strcat(work_path, "/dmenu.cfg");
            free_erase(namelist[i]);
			if ( access(work_path, F_OK) == 0 ) conf_merge_standalone(work_path);
        }
        free_erase(namelist);
    }
    
    //Process selectordir items
    i = cfg_size(cfg_main, "SelectorDir");
    while (i) {
        tmp = cfg_getnsec(cfg_main, "SelectorDir", --i);
        char* dir = cfg_getstr(tmp, "Dir");
        char* keys[3] =  {NULL, NULL, NULL };
        for (j=0;j<3;j++) keys[j] = cfg_getnstr(tmp, "Key", j);
        conf_set_item_path(cfg, keys, "SelectorDir", dir);
    }
}

void conf_reload_theme(char* theme)
{
    free_erase(THEME_CONF_FILE);
    cfg_free(cfg);
    conf_load_theme(theme);
    conf_process_theme();
}

int conf_load()
{
    log_debug("Initializing");

    // load dmenu.ini
    cfg_main = conf_from_file(main_opts, DMENU_CONF_FILE);
    if (cfg_main == NULL) return CFG_PARSE_ERROR;
    if (cfg_getbool(cfg_main, "ReadOnly"))
    {
        log_debug("Setting Readonly via Config");
        set_write_fs(0);
    }
    
    //Find theme path
    int rc = conf_load_theme(cfg_getstr(cfg_main, "Theme"));
    if (rc) return rc;
    
    conf_process_theme();

    return CFG_SUCCESS;
}

void conf_unload()
{
    log_debug("De-initializing");
    
    free_erase(THEME_CONF_FILE);
    cfg_free(cfg);

    if (can_write_fs()) {
		if (conf_changed ==  1) {
			conf_to_file(cfg_main, DMENU_CONF_FILE);
		} else {
            log_debug("Config was not changed");
        }
	}
    cfg_free(cfg_main);
    
    change_dir(DMENU_PATH);
}

int path_filter(const struct dirent *dptr)
{
    if (dptr->d_name[0] == '.') return 0;
    else return 1;
}

void conf_merge_standalone(char *conf_file)
{
    int i, j, number_of_menu, number_of_standalone_menu;
    cfg_t *standalone_cfg;
    cfg_t* m;
    cfg_t* standalone_m;
    char* conf_file_dir;

    standalone_cfg = conf_from_file(standalone_opts, conf_file);
    if (standalone_cfg == NULL) return;

    conf_file_dir = dirname(conf_file);

    number_of_standalone_menu = cfg_size(standalone_cfg, "Menu");
    number_of_menu = cfg_size(cfg, "Menu");

    for (i=0;i<number_of_standalone_menu;i++) {
        standalone_m = cfg_getnsec(standalone_cfg, "Menu", i);
        for (j=0;j<number_of_menu;j++) {
            m = cfg_getnsec(cfg, "Menu", j);
            if (strcmp(cfg_title(m), cfg_title(standalone_m)) == 0) {

                cfg_t* standalone_mi = cfg_getsec(standalone_m, "MenuItem");
                // Don't include this MenuItem if it includes SubMenuItems,
                // Our dupopts routine does not handle sections, hence doesn't
                // doesn't work for SubMenuItems
                if (cfg_size(standalone_mi, "SubMenuItem") > 0) {
                    log_error("Not supported - dmenu.cfg contains SubMenuItems " 
                                "in directory %s", conf_file_dir);
                }
                else {
                    char* icon_path = cfg_getstr(standalone_mi, "Icon");
                    if (icon_path[0] != '/') { // Icon path is relative
                        char work_path[PATH_MAX];
                        strcpy(work_path, conf_file_dir);
                        strcat(work_path, "/");
                        strcat(work_path, icon_path);
                        cfg_setstr(standalone_mi, "Icon", work_path);
                    }

                    cfg_opt_t* standalone_mi_opt = cfg_getopt(standalone_m, "MenuItem");
                    cfg_opt_t* mi_opt = cfg_getopt(m, "MenuItem");
                    mi_opt->values = realloc(mi_opt->values,
                                          (mi_opt->nvalues+1) * sizeof(cfg_value_t *));
                    mi_opt->values[mi_opt->nvalues] = (cfg_value_t*) malloc(sizeof(cfg_value_t));
                    mi_opt->values[mi_opt->nvalues]->section = calloc(1, sizeof(cfg_t));
                    cfg_t* mi_sec = mi_opt->values[mi_opt->nvalues]->section;
                    mi_opt->nvalues++;
                    cfg_t* s_mi_sec = standalone_mi_opt->values[0]->section;
                    mi_sec->name = strdup(s_mi_sec->name);
                    mi_sec->flags = s_mi_sec->flags;
                    mi_sec->filename = s_mi_sec->filename ? strdup(s_mi_sec->filename) : 0; 
                    mi_sec->line = s_mi_sec->line;
                    mi_sec->errfunc = s_mi_sec->errfunc;
                    mi_sec->title = s_mi_sec->title; 
                    mi_sec->opts = conf_dupopts(s_mi_sec->opts);
                }
            }
        }
    }

    cfg_free(standalone_cfg);
}

cfg_opt_t* conf_dupopts(cfg_opt_t* opts)
{
    int i, j;
    cfg_opt_t *dupopts;
    int n;
    for(n = 0; opts[n].name; n++) // to get number of opts
        /* do nothing */ ;

    dupopts = calloc(n+1, sizeof(cfg_opt_t));
    memcpy(dupopts, opts, n * sizeof(cfg_opt_t));

    for(i = 0; i < n; i++)
    {
        dupopts[i].name = strdup(opts[i].name);

        dupopts[i].values = calloc(dupopts[i].nvalues, sizeof(cfg_value_t*));
        for(j = 0; j < dupopts[i].nvalues; j++) {
            dupopts[i].values[j] = calloc(1, sizeof(cfg_value_t));
            switch (dupopts[i].type) {
              case CFGT_INT:
                dupopts[i].values[j]->number = opts[i].values[j]->number;
                break;
              case CFGT_FLOAT:
                dupopts[i].values[j]->fpnumber = opts[i].values[j]->fpnumber;
                break;
              case CFGT_BOOL:
                dupopts[i].values[j]->boolean = opts[i].values[j]->boolean;
                break;
              case CFGT_STR:
                if (opts[i].values[j]->string)
                    dupopts[i].values[j]->string = strdup(opts[i].values[j]->string);
                else
                    dupopts[i].values[j]->string = NULL;
                break;
              default: //we are not handling the other types
                break;
            }
        }

        if(opts[i].type == CFGT_SEC && opts[i].subopts)
            dupopts[i].subopts = conf_dupopts(opts[i].subopts);

        if(is_set(CFGF_LIST, opts[i].flags) || opts[i].type == CFGT_FUNC)
            dupopts[i].def.parsed = opts[i].def.parsed ? strdup(opts[i].def.parsed) : 0;
        else if(opts[i].type == CFGT_STR)
            dupopts[i].def.string = opts[i].def.string ? strdup(opts[i].def.string) : 0;
    }

    return dupopts;
}

int conf_set_item_path(cfg_t* config, char** titles, char* attr, char* val)
{
    
    int number_of_menu, number_of_menuitem, number_of_submenuitem,  i, j, k;
    cfg_t *m, *mi, *smi;  
    
    int depth = 0;
    for (i=1;i<3;i++) {
        if (titles[i] != NULL && strcmp(titles[i],"")!=0) depth++;
    }
    
    number_of_menu = cfg_size(config, "Menu");
    for (i=0;i<number_of_menu;i++) {
        m = cfg_getnsec(cfg, "Menu", i);
        if (m->title == NULL || strcmp(m->title, titles[0]) != 0) {
            continue;
        } else if (depth == 0) {
            cfg_setstr(m, attr, val);
            return 0;
        }
        
        number_of_menuitem = cfg_size(m, "MenuItem");
        for (j=0;j<number_of_menuitem;j++) {
            mi = cfg_getnsec(m, "MenuItem", j);
            if (mi->title == NULL || strcmp(mi->title, titles[1]) != 0) {
                continue;
            } else if (depth == 1) {
                cfg_setstr(mi, attr, val);
                return 0;
            }
            
            number_of_submenuitem = cfg_size(mi, "SubMenuItem");
            for (k=0;k<number_of_menuitem;k++) {
                smi = cfg_getnsec(mi, "SubMenuItem", k);
                if (smi->title == NULL) continue;
                if (strcmp(smi->title, titles[2]) == 0) {
                    cfg_setstr(smi, attr, val);
                    return 0;
                }
            }
        }
    }
    return 1;
}

char** conf_get_item_path(cfg_t* item) {
    
    char** names = new_str_arr(3); names[0] = names[1] = names[2] = "";
    int number_of_menu, number_of_menuitem, number_of_submenuitem, i, j, k;
    cfg_t *m, *mi, *smi;  

    number_of_menu = cfg_size(cfg, "Menu");
    for (i=0;i<number_of_menu;i++) {
        m = cfg_getnsec(cfg, "Menu", i);
        if (m == item) {
            names[0] =  m->title;
            return names;
        }
        
        number_of_menuitem = cfg_size(m, "MenuItem");
        for (j=0;j<number_of_menuitem;j++) {
            mi = cfg_getnsec(m, "MenuItem", j);
            if (mi == item) {
                names[0] = m->title;
                names[1] = mi->title;
                return names;
            }
            
            number_of_submenuitem = cfg_size(mi, "SubMenuItem");
            for (k=0;k<number_of_menuitem;k++) {
                smi = cfg_getnsec(mi, "SubMenuItem", k);
                if (smi == item) {
                    names[0] = m->title;
                    names[1] = mi->title;
                    names[2] = smi->title;
                    return names;
                }
            }
        }
    }
    return NULL;
}

void conf_themeselect(char* themedir)
{
    char* theme, *theme_name;
    if (strrchr(themedir, '.') != NULL) {
        theme = strndup(themedir, strrpos(themedir, '/')); //Strip off filename
    } else {
        theme = strdup(themedir);
    }
    
    if (strrchr(theme, '/')) {
        theme_name = strrchr(theme, '/')+1;
    } else {
        theme_name = theme;
    }
    
    log_debug("Setting theme: %s", theme_name);
    cfg_setstr(cfg_main, "Theme", theme_name);

    if (can_write_fs()) 
    {
        conf_to_file(cfg_main, DMENU_CONF_FILE);
    }
    
    strcpy(THEME_NAME, theme_name);
    reload(RELOAD_THEME);
    
    //Clean up
    free_erase(theme);
}

void conf_backgroundselect(char* bgimage)
{
    log_debug("Setting background image: %s", bgimage);
    cfg_setstr(cfg_main, "Background", bgimage);
    
    if (can_write_fs()) 
    {
        conf_to_file(cfg_main, DMENU_CONF_FILE);
    }
    
    menu_reload_background();
}

void conf_colorselect(char* color)
{
    log_debug("Setting font color: %s", color);
    cfg_setstr(cfg_main, "FontColor", color);
   
    if (can_write_fs()) 
    {
        conf_to_file(cfg_main, DMENU_CONF_FILE);
    }
	watch_color_reset();
	dosd_color_reset();
    
    reload(RELOAD_MENU);
}

void conf_dirselect(cfg_t* menu_item, char* dir) 
{    
    cfg_t* selector;
    cfg_setstr(menu_item, "SelectorDir", dir);

    int i =0;
    char key[PATH_MAX];
    char** keys = conf_get_item_path(menu_item);
    
    //Build key
    sprintf(key, "{\"%s\",\"%s\",\"%s\"}", keys[0], keys[1], keys[2]);
    
    //Update if possible
    int cnt = cfg_size(cfg_main, "SelectorDir");
    for (i=0;i<cnt;i++) 
    {
        selector = cfg_getnsec(cfg_main, "SelectorDir", i);
        
        if ((strcmp(cfg_getnstr(selector, "Key",0), keys[0]) == 0) &&
            (strcmp(cfg_getnstr(selector, "Key",1), keys[1]) == 0) &&
            (strcmp(cfg_getnstr(selector, "Key",2), keys[2]) == 0))
        {
            cfg_setstr(selector, "Dir", dir);
            break;
        }
    }
    
    free_erase(keys);

    //Persist data
    FILE* fp = open_conf_file(cfg_main, DMENU_CONF_FILE ".tmp");
    if (fp == NULL) return ;
    if (i == cnt)  //If not found
    {
        fprintf(fp, "SelectorDir {\nDir = \"%s\"\nKey = %s\n}\n", dir, key);
    }
    close_conf_file(fp);
    move_file(DMENU_CONF_FILE ".tmp", DMENU_CONF_FILE);
    
    if (i == cnt)
    {
        free_erase(cfg_main);
        cfg_main = conf_from_file(main_opts, DMENU_CONF_FILE);
    }
}
