#include <limits.h>

// the system ARG_MAX might be too large. just use a fixed
// value here.
#define MAX_CMD_LEN   4096
#define TICK_INTERVAL 50 // this is 1000/50 = 20 fps

//Currently the hardcoded defaults are intended for Dingoo A320
#ifndef DMENU_PATH
#define DMENU_PATH           "/mnt/mmc/dmenu/"
#endif 

#ifndef DMENU_CONF_FILE_NAME
#define DMENU_CONF_FILE_NAME "dmenu.ini"
#endif

#ifndef SOUND_ENABLED
#define SOUND_ENABLED        1
#endif

int  FILESYSTEM_READ_ONLY;
#define set_write_fs(b)      FILESYSTEM_READ_ONLY = !b
#define can_write_fs()       !FILESYSTEM_READ_ONLY
#define cant_write_fs()      FILESYSTEM_READ_ONLY

#define DIMMER_DELAY        60
#define SHUTDOWN_DELAY		0
#define DMENU_THEMES         DMENU_PATH "themes/"
#define DMENU_CONF_FILE      DMENU_PATH DMENU_CONF_FILE_NAME
#define DMENU_CONF_TEMP     "/tmp/.tmp"
#define DMENU_COMMAND_FILE	"/tmp/.nextCMD"
#define DMENU_SNAPSHOT		"/tmp/.screen"
#define DMENU_BACKGROUNDS    DMENU_PATH "wallpapers/"
#define GLOBAL_RESOURCE_PATH DMENU_PATH "resources/"
#define DMENU_THEME_MISSING  GLOBAL_RESOURCE_PATH "notfound.png"
#define THUMBNAILS_PATH      "/.thumb"
#define GLOBAL_RESOURCE_PRE  "global:"

char THEME_PATH[PATH_MAX];
char THEME_NAME[100];

//Device Info
#define CPU_DEVICE			"/proc/jz/cgm"
#define BACKLIGHT_DEVICE     "/proc/jz/lcd_backlight"
#define MIXER_DEVICE         "/dev/mixer"
#define BATTERY_DEVICE       "/proc/jz/battery"
#define USB_DEVICE       "/sys/devices/platform/musb_hdrc.0/uh_cable"


#if RS97
#define SCREEN_WIDTH         320
#define SCREEN_HEIGHT        240
#define PREVIEW_OFFSET_RIGHT 100
#else
#define SCREEN_WIDTH         480
#define SCREEN_HEIGHT        272
#define PREVIEW_OFFSET_RIGHT 240
#endif


#define SCREEN_COLOR_DEPTH   16
#define SCREEN_BPP           (SCREEN_COLOR_DEPTH>>3)

#define MENU_ACTIVE_ALPHA    255
#define MENU_INACTIVE_ALPHA  127
#define MENU_TEXT_FONT_SIZE  18
#define MENU_TEXT_HEIGHT     (MENU_TEXT_FONT_SIZE+2)
#define MENU_ITEM_FONT_SIZE  13


#define SELECT_TITLE_HEIGHT  20
#define SELECT_TITLE_ALPHA   196
#define SELECT_TITLE_COLOR   0x989898
#define SELECT_BG_COLOR      0x303030

//Filelist options
#define FILE_LIST_OFFSET     24
#define FILE_ENTRY_HEIGHT    27
#define FILES_PER_PAGE       ((SCREEN_HEIGHT - FILE_LIST_OFFSET)/FILE_ENTRY_HEIGHT)

//Imageview options
#define IMAGE_THUMB_RATIO_OUTER .25
#define IMAGE_THUMB_RATIO_INNER .225
#define IMAGE_PREVIEW_RATIO     .7
#define IMAGE_THUMBS_PER_PAGE   (1/IMAGE_THUMB_RATIO_OUTER)
#define IMAGE_THUMB_HEIGHT      (IMAGE_THUMB_RATIO_OUTER*SCREEN_HEIGHT)
#define IMAGE_THUMB_WIDTH       (IMAGE_THUMB_RATIO_OUTER*SCREEN_WIDTH)
#define IMAGE_THUMB_PAD_X       ((SCREEN_WIDTH*IMAGE_THUMB_RATIO_OUTER-SCREEN_WIDTH*IMAGE_THUMB_RATIO_INNER)/2.0)
#define IMAGE_THUMB_PAD_Y       ((SCREEN_HEIGHT*IMAGE_THUMB_RATIO_OUTER-SCREEN_HEIGHT*IMAGE_THUMB_RATIO_INNER)/2.0)
#define IMAGE_THUMB_TOP         (SCREEN_HEIGHT-IMAGE_THUMB_HEIGHT-2)
#define IMAGE_SELECT_COLOR      0xFFFFFF
#define IMAGE_SELECT_ALPHA      64

//#define DOSD_COLOR           0xFFFFFF
#define DOSD_UPDATE_INTERVAL 1000
#define DOSD_PADDING         4
#define DOSD_BATTERY_WIDTH   19
#define DOSD_LOCK_WIDTH      7
#define DOSD_HEIGHT          9
#define DOSD_START_X         (SCREEN_WIDTH-DOSD_PADDING*2-DOSD_BATTERY_WIDTH-DOSD_LOCK_WIDTH)

//Volume Positioning
#define VOLUME_ICON_MIN_W    9
#define VOLUME_ICON_MAX_W    30
#define VOLUME_ICON_H        DOSD_HEIGHT
#define VOLUME_ICON_Y        DOSD_PADDING
#define VOLUME_ICON_X        (DOSD_START_X- VOLUME_ICON_MAX_W - DOSD_PADDING)

//ColorPicker
#define COLORPICKER_PAD             8
#define COLORPICKER_SELECT_INACTIVE 0xFFFFFF,0x88
#define COLORPICKER_SELECT_ACTIVE   0xFFFFFF,0xFF
#define COLORPICKER_COLOR_H         15
#define COLORPICKER_COLOR_W         255
#define COLORPICKER_COLOR_FULL      (COLORPICKER_COLOR_H+COLORPICKER_PAD)
#define COLORPICKER_COLOR_X         (SCREEN_WIDTH/2-COLORPICKER_COLOR_W/2)
#define COLORPICKER_COLOR_Y         (SCREEN_HEIGHT-COLORPICKER_COLOR_FULL*3)

#define COLORPICKER_PREVIEW_W       (COLORPICKER_COLOR_W + COLORPICKER_PAD*2)
#define COLORPICKER_PREVIEW_H       (COLORPICKER_COLOR_Y - SELECT_TITLE_HEIGHT - COLORPICKER_PAD*2)
#define COLORPICKER_PREVIEW_Y       (SELECT_TITLE_HEIGHT + COLORPICKER_PAD)
#define COLORPICKER_PREVIEW_X       (SCREEN_WIDTH/2 - COLORPICKER_PREVIEW_W/2)

//Brightness positioning
#define BRIGHTNESS_ICON_W 9
#define BRIGHTNESS_ICON_H DOSD_HEIGHT
#define BRIGHTNESS_ICON_X (VOLUME_ICON_X - BRIGHTNESS_ICON_W - DOSD_PADDING)
#define BRIGHTNESS_ICON_Y DOSD_PADDING

//Watch positioning
#define WATCH_DISP_X	2
#define WATCH_DISP_Y	1
#define WATCH_FONT_SIZE	12

//MHz display positioning
#define CPU_DISP_X		175
#define CPU_DISP_Y		1
#define CPU_FONT_SIZE	12

