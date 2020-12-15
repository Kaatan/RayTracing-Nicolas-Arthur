#include <cstdio>
#include <iostream>

#include <SDL2/SDL.h>

using namespace std;

int main(int argc, char** args) {

	// Pointers to our window and surface
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;

	// Initialize SDL. SDL_Init will return -1 if it fails.
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	}

	// Create our window
	window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);

	// Make sure creating the window succeeded
	if (!window) {
		cout << "Error creating window: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	}

	// Get the surface from the window. Those are now linked.
	winSurface = SDL_GetWindowSurface(window);

	// Make sure getting the surface succeeded
	if (!winSurface) {
		cout << "Error getting surface: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	}
    
	// Fill the window with a black rectangle for testing
	SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 0, 0, 0));
    
	// Update the window display
	SDL_UpdateWindowSurface(window);

    
    SDL_Delay(1000);//Short wait for good measure
    

    SDL_Surface* gHelloWorld = SDL_LoadBMP( "hello_world.bmp" ); //Getting an image into a screen
	if( gHelloWorld == NULL )//Checking image reception
	{
		printf( "Unable to load image %s! SDL Error: %s\n", "hello_world.bmp", SDL_GetError() );
		return 1;
	}


    //Apply the image to the screen of the window
    SDL_BlitSurface( gHelloWorld, NULL, winSurface, NULL );

	// Update the window display
	SDL_UpdateWindowSurface(window);

	// Wait for a input in the cmd
	system("pause");

	// Destroy the window. This will also destroy the surface
	SDL_DestroyWindow(window);

	// Quit SDL
	SDL_Quit();

	// End the program
	return 0;
}