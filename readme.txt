This fork is based on Dmenu 0.7(test release) and adapted to work on the PAP KIII Plus (K3P)     





									   Dmenu 
                               =======================

Dmenu is a simple menu system for Dingux (Linux on K3P and other). It is optimized to work with the 480x272
resoltion of the K3P. You need an sd-card for this menu that plugs into the external slot of the K3P.



Installation
------------
Advice: Make a backup of your internal sd-card! That means openig the device, removing the internal card and making a backup with a software like "win32diskimager" on windows or dd on linux. This step is not required, but the installation of this dmenu will alter internal partitions and having a backup is a good idea!

1. Extract the content of the lates release(e.g. dmenu_0.7_k3_0.5.zip ) onto your new external card. You need the dmenu folder in the root of your sd-card
2. If you want to use the alternative emulators as well (you do!), extract the emulators.zip and apps.zip to the root folder as well.
3. Use the build-in file browser and navigate to sd-card/dmenu/installer
4. Run the install.dge file

-> The new dmenu should open and is now the default menu.

To uninstall the dmenu, just remove the external sd-card or delete/rename the dmenu folder on the external card. If the folder is not found,
the old (default) menu is loaded.

SwitchMenu.dge in the dmenu folder allows to switch between the old and this dmenu. 


Keys
----
In main menu,

left, right key - select menu
left, B key - close SubMenu if you opened
up, down key - select menu item
A key - run the selected menu item / open SubMenu
Select + Start key - exit dmenu
R key - SoundVolume +5% (or volume key)
L key - SoundVolume -5% (or volume key)
X key - Brightness level up
Y key - Brightness level down

In file selector menu,

up, down key - select file
left  - change to the parent directory
right - go to the selected directory
A key - run the selected file or go to the selected directory
B key - exit file selector menu
C key - toggle preview (default enabled)
L, R key - page up/down
Start - Save current directory as the future value of
        SelectorDir for the menuitem that called it

In color selector menu,

up, down key - select R/G/B
left  - reduce selected color value broadly
right - increase selected color value broadly
L key - reduce selected color value finely
R key - increase selected color value finely
A key - set color and exit color selector menu
B key - exit color selector menu

Theme support
-------------
Dmenu on startup reads /usr/local/dmenu/dmenu.ini, which contains a line to 
specify the theme in the form of "Theme = <theme name>".

Dmenu reads /usr/local/dmenu/themes/<theme name>/theme.cfg to load the theme.


Theme config file includes
--------------------------
Theme config file can include other config files. See theme.cfg in the sample 
theme called 'include_sample'.


In the individual dmenu.cfg files, if the icon file path is relative (i.e. the 
first char is not '/'), Dmenu will treat this as relative to the directory where
this dmenu.cfg is in.


SubMenu
-------
To add a submenu, just add a menuitem without executable and workpath, but with
SubMenuItems. For example,

    MenuItem SubMenuTest
    {
        Icon = "res/game1.png"
        Name = "SubMenuTest"

        SubMenuItem SubTest1
        {
            Icon = "res/game2.png"
            Name = "SubTest1"
            Executable = "./duh"
            WorkDir = "/usr/local/duh"
        }
        SubMenuItem SubTest2
        {
            Icon = "res/game2.png"
            Name = "SubTest2"
            Executable = "./duh"
            WorkDir = "/usr/local/duh"
        }
    }

Once the submenu is selected, press A to open it, and press B to close. Use 
up/down to select the submenu items and press A to launch.

One caveat though, dmenu.cfg included from SearchPath does not support submenu.
dmenu will print a warning and ignore the menuitem if it's a submenu.

Selector and SelectorDir
------------------------
In MenuItem and SubMenuItem, you can use Selector and SelectorDir to get into
the filelist. For example,

    MenuItem foo
    {
        Icon = "res/emu1.png"
        Name = "SomeEmulator"
        Executable = "./emu"
        WorkDir = "/usr/local/emulator/emu"
        Selector = "true"
        SelectorDir = "/usr/local/data/roms"	
    }

The filelist current folder is set to "SelectorDir". If the user has set
SelectorDir manually for a MenuItem (by pressing Start in the fileselector),
that value will override the MenuItem value.

Other options
------------------------
	  MenuItem foo
    {
        Icon = "res/emu1.png"
        Name = "SomeEmulator"
        Executable = "./emu"
        WorkDir = "/usr/local/emulator/emu"
        Selector = "true"
        SelectorDir = "/usr/local/data/roms"
		Previews = "/usr/local/data/previews/"
        Extensions = "gba zip"
		Center = true
		VolumeControl = true
    }

Previews(Optional) - Previews defines a folder where preview images can be found. Only png files are supported, named exactly 
		   like the rome without extension (rom.zip mapps to rom.png). The images have a fixed postion, use max 200px wide 
		   images for the best results.

Extensions(Optional) - Extensions defines a list of extension which files should be listed in the selector dir. Files
			 with extensions not in this list will not be visible. (Gets rid of the savestate files in rom folders)

Center/Zomm(Optional) - The Center or Zoom parameters are used to provide a fallback for older dingux software that is not patched 
			  to use the larger resolution. If parameter Center = true, the 320x240 window of the executed software is centered on the screen. If Zoom = true, the 320x240 are stretched to fullscreen. Zoom comes with some performace issues. 

VolumeControl(Optional) - VolumeControl is a legacy option for software not directly written for the K3P, but supporting the resolution.
				The K3P hardware buttons are unique for this device and not supported by most applications. By enabling this option, a helper is 
				started that enables these buttons to work for all applications. 

 


Theme Selector with thumbnail
-----------------------------
In the menu, you can utilize the builtin Theme Selector menuitem.
For example,

    MenuItem ThemeSelect
    {
        Icon = "res/star1.png"
        Name = "Select Theme"
        Executable = "!themeselect"
    }

The code will automatically scan the themes folder and determine
the theme preview from an image file in the theme folder.  
<dmenu_root>/themes/<theme_name>/theme.png is the file that will
be used to determine the theme preview.  If the preview.png is not
found then resources/notfound.png is used.

Wallpaper Selector with thumbnail
---------------------------------
In Menu, you can make "built in Wallpaper Selector" menuitem.
For example,

    MenuItem BackgroundSelect
    {
        Icon = "res/star1.png"
        Name = "Select Wallpaper"
        Executable = "!backgroundselect"
    }

You can add your favorite wall paper to "/usr/local/dmenu/wallpapers".
WallpaperSelecter pick up all png file automatically from that folder.



FontColor Selector
------------------
In Menu, you can make "FontColor Selector" menuitem.
For example,

    MenuItem FontColor
    {
        Icon = "res/tri4.png"
        Name = "FontColor"
        Executable = "!colorselect"
    }



UTF-8
-----
To display UTF8 characters, font file specified in theme.cfg should contain the 
necessary characters. UTF8 chars in theme.cfg name strings, and filenames in 
file selector will be displayed accordingly.



Building dmenu
--------------
There are 2 makefiles in dmenu source distribution, Makefile.host and
Makefile.dingoo.

Makefile.host will build dmenu to run on host pc running linux. This is a 
standard SDL application. You will need the SDL development libraries and
libconfuse to build.

Makefile.dingoo is the makefile for building dmenu to run in Dingux. You need a
toolchain (https://sience.schattenkind.net/dingoo/Dingux_Toolchain_for_Windows.htm#Libraries) to build this.
All the required libraries are already included in the toolchain.

To make building more easy, two simple scripts (clean_release.sh and build_release.sh) are included
