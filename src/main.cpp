#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include "shapes.h"
#include "ray.h"

#include "csv_reader.cpp"

#include "Eigen/Dense"

#include <SDL2/SDL.h>

#include <OMP.h>


using namespace std;
using namespace Eigen;

#define WIDTH 1280
#define HEIGHT 720


void renderAll(Vector3f* pixels, int Width, int Height, SDL_Renderer* SdlRenderer, SDL_Texture* m_texture){
	printf("Entered rendering\n");
	SDL_SetRenderTarget(SdlRenderer, m_texture);
	SDL_SetRenderDrawColor(SdlRenderer, 255, 255, 255, 255); //Sélectionne une couleur blanche
	SDL_RenderClear(SdlRenderer); //Colorie tout en blanc
    
	
	for (int i = 0; i < Height; i++){
		for (int j = 0; j < Width; j++){
			if (pixels[j+ Width*i](0) != 2.0f || pixels[j+ Width*i](1) != 2.0f ||pixels[j+ Width*i](2) != 2.0f){
				SDL_SetRenderDrawColor(SdlRenderer, (Uint8) (255 * pixels[j+ Width*i](0)), (Uint8) (255 * pixels[j+ Width * i](1)), (Uint8) (255 * pixels[j + Width * i](2)), 255);
				//printf("Couleurs du pixel :  %f  %f  %f", pixels[j+ Width*i](0), pixels[j+ Width * i](1), pixels[j + Width * i](2));
				//SDL_SetRenderDrawColor(SdlRenderer, 255, 0, 0, 255); //Sélectionne une couleur rouge
				SDL_RenderDrawPoint(SdlRenderer, j, i);
			}
		}
	}
	SDL_RenderCopy(SdlRenderer, m_texture, NULL, NULL);
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

void addSphereFromLine(std::vector<std::string> elem, vector<Sphere> &spheres) {
	try {
		float x,y,z; //Sphere position
		float R; //Radius
		float r,g,b; //Sphere color
		float reflectivity, transparency;
		float er, eg, eb; //Emission color

		x = stof(elem[0]);
		y = stof(elem[1]);
		z = stof(elem[2]);
		R = stof(elem[3]);
		r = stof(elem[4]);
		g = stof(elem[5]);
		b = stof(elem[6]);
		reflectivity = stof(elem[7]);
		transparency = stof(elem[8]);
		er = stof(elem[9]);
		eg = stof(elem[10]);
		eb = stof(elem[11]);

		spheres.push_back(Sphere(Vector3f( x, y, z), R, Vector3f(r, g, b), reflectivity, transparency, Vector3f(er,eg,eb))); 
	}
	catch(...) {cout << "Incorrect parameters";}
}

void addSpheresFromFile(string fileName, vector<Sphere> &spheres) {
    std::filebuf fb;
    if (fb.open (fileName,ios::in))
    {
        istream is(&fb);
        std::vector<std::vector<std::string>> table = readCSV(is);
		table.erase(table.begin());
		for (std::vector<std::string> elem : table) {
			addSphereFromLine(elem, spheres);
		}
        fb.close();
    }

}




int main(int argc, char** args) {
	
	SDL_Window* window = NULL;
	SDL_Renderer* sdlRenderer = NULL;
	SDL_Texture* sdlTexture = SDL_CreateTexture( sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
	//IMG_LoadTexture(m_renderer, "data/galaxie.jpg"); //changer le fichier pour un fond blanc


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

	//printf("Finished SDL init\n");



	vector<Sphere> spheres; 


	//printf("Finished empty Sphere vector creation\n");
	
	addSpheresFromFile("spheres.csv", spheres);
	cout << spheres.size() << endl;

	vector<Cube> cubes;

	printf("Finished Cube vector creation\n");
	
	//Vector3f pixels[WIDTH*HEIGHT];

	vector<Vector3f> pixels(WIDTH*HEIGHT);

	printf("Finished pixels table creation\n");

	chrono::steady_clock::time_point start, mid, mmid, end;


		start = chrono::steady_clock::now();

		ComputeAll(spheres, cubes, WIDTH, HEIGHT, pixels.data());

		printf("Finished computing");

		mid = chrono::steady_clock::now();

		renderAll(pixels.data(), WIDTH, HEIGHT, sdlRenderer, sdlTexture);

		mmid = chrono::steady_clock::now();

		SDL_RenderPresent(sdlRenderer);//mise à jour de l'écran

		end = chrono::steady_clock::now();
		chrono::duration<double> global_elaps = end - start;
		chrono::duration<double> calc_elaps = mid - start;
		chrono::duration<double> ccalc_elaps = mmid - mid;
		chrono::duration<double> render_elaps = end - mmid;

		cout <<"Computing time " << calc_elaps.count()*1000 <<  "  Rendering time " << render_elaps.count()*1000 << "  Total time " <<global_elaps.count()*1000 << ", FPS " << 1/global_elaps.count() << std::endl;
		cout << "Rendering calculation time " << ccalc_elaps.count()*1000 << endl;
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