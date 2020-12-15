#include <cstdio>
#include <iostream>

#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;



bool init(SDL_Window* gWindow, SDL_Surface* gScreenSurface) //Initialisation de la fenêtre et de la surface
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Get window surface
			gScreenSurface = SDL_GetWindowSurface( gWindow );
		}
	}

	return success;
}



bool loadMedia(SDL_Window* gWindow, SDL_Surface* gScreenSurface) //Introduction d'une image dans la surface
{
	//Loading success flag
	bool success = true;

	//Load splash image
	gScreenSurface = SDL_LoadBMP( "hello_world.bmp" );
	if( gScreenSurface == NULL )
	{
		printf( "Unable to load image %s! SDL Error: %s\n", "02_getting_an_image_on_the_screen/hello_world.bmp", SDL_GetError() );
		success = false;
	}

	return success;
}



void close(SDL_Window* gWindow, SDL_Surface* gScreenSurface) //Fermeture de l'image, de la surface et de la fenêtre
{
	//Deallocate surface
	SDL_FreeSurface( gScreenSurface );
	gScreenSurface = NULL;

	//Destroy window
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}



int main( int argc, char* argv[] )
{
    std::cout << "test" << std::endl;
    //The window we'll be rendering to
    SDL_Window* gWindow = NULL;

    //The surface contained by the window
    SDL_Surface* gScreenSurface = NULL;
    //We'll update the surface before updating it to the window

    //The image we will load and show on the screen
    SDL_Surface* gHelloWorld = NULL;

	//Start up SDL and create window
	if( !init(gWindow, gScreenSurface) )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia(gWindow, gHelloWorld) )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{
			//Apply the image
			SDL_BlitSurface( gHelloWorld, NULL, gScreenSurface, NULL );
			
			//Update the surface
			SDL_UpdateWindowSurface( gWindow );

			//Wait two seconds
			SDL_Delay( 2000 );
		}
	}

	//Free resources and close SDL
    SDL_Delay( 2000 );
	close(gWindow, gScreenSurface);

	return 0;
}