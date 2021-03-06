#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include <cmath>

#include "shapes.h"
#include "ray.h"

#include "Eigen/Dense"

#include <SDL2/SDL.h>

#include <OMP.h>


using namespace std;
using namespace Eigen;

#define WIDTH 1080
#define HEIGHT 720


void renderAll(Vector3f* pixels, int Width, int Height, SDL_Renderer* SdlRenderer, SDL_Texture* m_texture){

	SDL_SetRenderTarget(SdlRenderer, m_texture);//Ecriture sur la texture liée au renderer
	unsigned char* texture_pixels;
	int pitch;

	SDL_LockTexture(m_texture, NULL, (void**)&texture_pixels, &pitch ); //Verrouillage de la texture pour l'éditer
	for (int i = 0; i < Height; i++){
		for (int j = 0; j < Width; j++){
			texture_pixels[4*(j + Width*i)] = 255;
			texture_pixels[4*(j + Width*i)+1] = (char) (255 * pixels[j+ Width*i](2));
			texture_pixels[4*(j + Width*i)+2] = (char) (255 * pixels[j+ Width*i](1));
			texture_pixels[4*(j + Width*i)+3] = (char) (255 * pixels[j+ Width*i](0));
		}
	} //Ecriture sur la texture

	SDL_UnlockTexture(m_texture); //Déverrouillage

	SDL_RenderCopy(SdlRenderer, m_texture, NULL, NULL); //Copie de la texture sur le renderer
}



void ComputeAll(vector<Sphere> &spheres, vector<Cube> &cubes, int width, int height, Vector3f* pixels){
	//modifie directement le tableau de vecteurs pixels

    //Définitions des paramètres
    float invWidth = 1 / float(width), invHeight = 1 / float(height); 
    float fov = 30, aspectratio = width / float(height); 
    float angle = tan(M_PI * 0.5 * fov / 180.); 
    
	// Traçage des rayons avec parallélisation
	#pragma omp parallel for simd
    for (int y = 0; y < height; ++y) { 
        for (int x = 0; x < width; ++x) { 
            //Calcul de l'angle d'envoi du rayon initial
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio; 
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle; 
			
			//Création d'un objet rayon
            Vector3f raydir(xx, yy, -1); 
            raydir.normalize(); 
			
			//Traçage du rayon
            pixels[x + width*y] = trace_sphere(Vector3f(0,0,0), raydir, spheres, 0); 
			
        } 
    } 
    
    return;
}



int main(int argc, char** args) {
	
	//Initialisation de SDL2
	SDL_Window* window = NULL;
	SDL_Renderer* sdlRenderer = NULL;
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		return 1;
	}
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
    {
        printf("Warning: Linear texture filtering not enabled!");
        return 1;
    }



	//Création d'une fenêtre
	window = SDL_CreateWindow("Affichage", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN); //Création de la fenêtre vidéo
	if (window == NULL) { //Vérifier si la fenêtre a bien été créée
        printf("Failed to create display with dimensions %dx%d : %s\n", WIDTH, HEIGHT, SDL_GetError());
        SDL_Quit();
        return 1;
    }



	sdlRenderer = SDL_CreateRenderer(window, -1, 0); //Création d'un renderer

	if (sdlRenderer == NULL) {
		printf("Failed to create renderer with dimensions %dx%d : %s\n", WIDTH, HEIGHT, SDL_GetError());
        SDL_Quit();
		return 1;
	}

	SDL_Texture* sdlTexture = SDL_CreateTexture( sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);



	vector<Sphere> spheres; 	
	addSpheresFromFile("spheres.csv", spheres); //Lecture du fichier csv pour ajouter les spheres

	vector<Cube> cubes;



	vector<Vector3f> pixels(WIDTH*HEIGHT); //Cration d'un tableau pour stocker les pixels qui serton affichés
	printf("Finished pixels table creation\n");



	chrono::steady_clock::time_point start, mid, mmid, end;

	chrono::duration<double> t = (chrono::duration<double>)0;

	while(1) {

		start = chrono::steady_clock::now();

		ComputeAll(spheres, cubes, WIDTH, HEIGHT, pixels.data()); //Calcul de l'affichage et exportation dans le tableau pixels

		mid = chrono::steady_clock::now();

		renderAll(pixels.data(), WIDTH, HEIGHT, sdlRenderer, sdlTexture); //Affichage sur la texture des données du tableau

		mmid = chrono::steady_clock::now();

		SDL_RenderPresent(sdlRenderer);//Mise à jour de l'écran

		end = chrono::steady_clock::now();



		chrono::duration<double> global_elaps = end - start;
		chrono::duration<double> calc_elaps = mid - start;
		chrono::duration<double> render_elaps = end - mid;
		t+= global_elaps;



		//Mouvement des sphères
		spheres[1].center(0)=1*sin(t.count()*5);
		spheres[2].center(0)=-10*sin(t.count()*5);
		spheres[2].center(2)=-20+10*cos(t.count()*5);



		cout <<"Compute time " << calc_elaps.count()*1000
				<<  ",  Render time " << render_elaps.count()*1000
				<< ",  Total time " << global_elaps.count()*1000
				<< ", FPS " << 1/global_elaps.count() << "\r"
				<< std::flush;
		
	}
	


	//Arrêt de SDL
	system("pause");
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}