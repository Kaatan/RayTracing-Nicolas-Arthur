#include <cstdio>
#include <iostream>
#include <vector>
#include <chrono>

#include "shapes.h"
#include "ray.h"

#include "Eigen/Dense"

#include <SDL2/SDL.h>

#include <OMP.h>


using namespace std;
using namespace Eigen;

#define WIDTH 1280
#define HEIGHT 720


void renderAll(Vector3f* pixels, int Width, int Height, SDL_Renderer* SdlRenderer){
	printf("Entered rendering\n");
	SDL_RenderClear(SdlRenderer);
	for (int i = 0; i < Height; i++){
		for (int j = 0; j < Width; j++){
			SDL_SetRenderDrawColor(SdlRenderer, (Uint8) (255 * pixels[j+ Width*i](0)), (Uint8) (255 * pixels[j+ Width * i](1)), (Uint8) (255 * pixels[j + Width * i](2)), 255);
			//printf("Couleurs du pixel :  %f  %f  %f", pixels[j+ Width*i](0), pixels[j+ Width * i](1), pixels[j + Width * i](2));
			SDL_RenderDrawPoint(SdlRenderer, j, i);
		}
	}
	SDL_RenderPresent(SdlRenderer);//mise à jour de l'écran
}

void ComputeAll(vector<Sphere> &spheres, vector<Cube> &cubes, int width, int height, Vector3f* pixels){
	//modofie directement le tableau de vecteurs pixels

	printf("Starting computations\n");
     
    float invWidth = 1 / float(width), invHeight = 1 / float(height); 
    float fov = 30, aspectratio = width / float(height); 
    float angle = tan(M_PI * 0.5 * fov / 180.); 
    // Trace rays

	#pragma omp parallel for simd
    for (int y = 0; y < height; ++y) { 
        for (int x = 0; x < width; ++x) { 
            
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio; 
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle; 
			//Calcul de l'angle d'envoie du rayon initial
			
            Vector3f raydir(xx, yy, -1); 
			//printf("Created raydir\n");
            raydir.normalize(); 
			// printf("Normalized raydir\n");
			//printf("Entering ")
            pixels[x + width*y] = trace_sphere(Vector3f(0,0,0), raydir, spheres, 0); 
			//pixels[x + width*y] = pixels[x + width*y] + trace_cube(Vector3f(0), raydir, cubes, 0);
        } 
    } 
    
    return;
}


int main(int argc, char** args) {
	
	SDL_Window* window = NULL;
	SDL_Renderer* sdlRenderer = NULL;



	// Initialize SDL. SDL_Init will return -1 if it fails.
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	}

	//Set texture filtering to linear
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
    {
        printf("Warning: Linear texture filtering not enabled!");
        return 1;
    }

	// Create our window
	window = SDL_CreateWindow("Affichage", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN); //Création de la fenêtre vidéo

	// Make sure creating the window succeeded
	if (window == NULL) {
        printf("Impossible de créer l'affichage de dimensions %dx%d : %s\n", WIDTH, HEIGHT, SDL_GetError());
        SDL_Quit();
        return 1;
    }

	sdlRenderer = SDL_CreateRenderer(window, -1, 0); //Création d'un truc pour render

	// Make sure getting the surface succeeded
	if (sdlRenderer == NULL) {
		printf("Impossible de créer le renderer de dimensions %dx%d : %s\n", WIDTH, HEIGHT, SDL_GetError());
        SDL_Quit();
		// End the program
		return 1;
	}
    
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255); //Sélection d'une couleur
	SDL_RenderClear(sdlRenderer); //Peinture de tout l'écran en Noir
	
    //SDL_Delay(1000);//Short wait for good measure
    SDL_RenderPresent(sdlRenderer);//mise à jour de l'écran


	//printf("Finished SDL init\n");



	vector<Sphere> spheres; 


	//printf("Finished empty Sphere vector creation\n");

    // position, radius, surface color, reflectivity, transparency, emission color
    spheres.push_back(Sphere(Vector3f( 0.0, -10004, -20), 10000, Vector3f(0.20, 0.20, 0.20), 0, 0.0, Vector3f(0,0,0))); 
    spheres.push_back(Sphere(Vector3f( 0.0,      0, -20),     4, Vector3f(1.00, 0.32, 0.36), 1, 0.5, Vector3f(0,0,0))); 
    spheres.push_back(Sphere(Vector3f( 5.0,     -1, -15),     2, Vector3f(0.90, 0.76, 0.46), 1, 0.0, Vector3f(0,0,0))); 
    spheres.push_back(Sphere(Vector3f( 5.0,      0, -25),     3, Vector3f(0.65, 0.77, 0.97), 1, 0.0, Vector3f(0,0,0))); 
    spheres.push_back(Sphere(Vector3f(-5.5,      0, -15),     3, Vector3f(0.90, 0.90, 0.90), 1, 0.0, Vector3f(0,0,0))); 

	//printf("Finished Shpere Objects addition\n");
    // light
    spheres.push_back(Sphere(Vector3f( 0.0,     20, -30),     3, Vector3f(0.00, 0.00, 0.00), 0, 0.0, Vector3f(3,3,3))); 

	//printf("Finished Light creation\n");

	vector<Cube> cubes;

	printf("Finished Cube vector creation\n");
	
	//Vector3f pixels[WIDTH*HEIGHT];

	vector<Vector3f> pixels(WIDTH*HEIGHT);

	printf("Finished pixels table creation\n");

	chrono::steady_clock::time_point start, mid, end;

    start = chrono::steady_clock::now();

	ComputeAll(spheres, cubes, WIDTH, HEIGHT, pixels.data());
	
	printf("Finished computing");
	
	mid = chrono::steady_clock::now();

	renderAll(pixels.data(), WIDTH, HEIGHT, sdlRenderer);

    end = chrono::steady_clock::now();
    chrono::duration<double> global_elaps = end - start;
    chrono::duration<double> calc_elaps = mid - start;
	chrono::duration<double> render_elaps = end - mid;
	
	cout <<"Computing time " << calc_elaps.count()*1000 << "  Rendering time " << render_elaps.count()*1000 << "  Total time" <<global_elaps.count()*1000 << ", FPS " << 1/global_elaps.count() << std::endl;

	//printf("Finished rendering");

	// Wait for a input in the cmd
	system("pause");

	// Destroy the window. This will also destroy the surface
	SDL_DestroyWindow(window);

	// Quit SDL
	SDL_Quit();

	// End the program
	return 0;
}