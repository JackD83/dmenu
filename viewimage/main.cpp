#include <iostream>
#include "SDL.h"
#include <SDL_image.h>

using std::cerr;
using std::endl;

int main(int argc, char *args[])
{
    // Initialize the SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        cerr << "SDL_Init() Failed: " << SDL_GetError() << endl;
        exit(1);
    }


    const SDL_VideoInfo* info = SDL_GetVideoInfo();   //<-- calls SDL_GetVideoInfo();   
    int screenWidth = info->current_w;
    int screenHeight = info->current_h;

    if(screenWidth > 480) {
        screenWidth = 320;
        screenHeight = 480;
    }


    // Set the video mode
    SDL_Surface *display;
    display = SDL_SetVideoMode(screenWidth, screenHeight, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (display == NULL)
    {
        cerr << "SDL_SetVideoMode() Failed: " << SDL_GetError() << endl;
        exit(1);
    }

    SDL_ShowCursor( SDL_DISABLE ); 

    // Load the image
    SDL_Surface *image;
    image = IMG_Load("usb.png");
    if (image == NULL)
    {
        cerr << " IMG_Load() Failed: " << SDL_GetError() << endl;
        exit(1);
    }

    // Main loop
    SDL_Event event;
    while (1)
    {    
        // Check for messages
        if (SDL_PollEvent(&event))
        {
            // Check for any key pressed
            if (event.type == SDL_KEYDOWN )
            {
                // Quit the program
                break;
            }
        }
        // Game loop will go here...
        // Apply the image to the display
        if (SDL_BlitSurface(image, NULL, display, NULL) != 0)
        {
            cerr << "SDL_BlitSurface() Failed: " << SDL_GetError() << endl;
            exit(1);
        }

        //Update the display
        SDL_Flip(display);
    }

    // Tell the SDL to clean up and shut down
    SDL_Quit();

    return 0;
}