#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include <cmath>

#include "shapes.h"
#include "ray.h"
#include "parameters.h"

#include "Eigen/Dense"

#include <SDL2/SDL.h>

#include <OMP.h>


using namespace std;
using namespace Eigen;


void renderAll(Vector3f* pixels, int width, int height, SDL_Renderer* SdlRenderer, SDL_Texture* m_texture){

	SDL_SetRenderTarget(SdlRenderer, m_texture);//Ecriture sur la texture liée au renderer
	unsigned char* texture_pixels;
	int pitch;

	SDL_LockTexture(m_texture, NULL, (void**)&texture_pixels, &pitch ); //Verrouillage de la texture pour l'éditer
	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++){
			texture_pixels[4*(j + width*i)] = 0;
			texture_pixels[4*(j + width*i)+1] = (char) (255 * pixels[j+ width*i](2));
			texture_pixels[4*(j + width*i)+2] = (char) (255 * pixels[j+ width*i](1));
			texture_pixels[4*(j + width*i)+3] = (char) (255 * pixels[j+ width*i](0));
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
	#pragma omp parallel for simd schedule(dynamic)
    for (int y = 0; y < height; ++y) { 
        for (int x = 0; x < width; ++x) { 
            //Calcul de l'angle d'envoi du rayon initial
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio; 
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle; 
			
			//Création d'un objet rayon
            Vector3f raydir(xx, yy, -1); 
            raydir.normalize(); 
			
			//Calcul des valeurs du pixel
			Vector3f pixel = trace_sphere(Vector3f(0,0,0), raydir, spheres, 0);

			//Si une des couleurs du pixel a une valeur supérieure à 1, on divise tout le vecteur pixel par cette valeur
			//Cela permet d'éviter un overflow après à la conversion en char
			float maxValue = pixel.maxCoeff();
			if (maxValue > 1) {
				pixel /= maxValue;
			}

			//Traçage du rayon
            pixels[x + width*y] = pixel; 
			
        } 
    } 
    
    return;
}


//Appelle ComputeAll() avec résolution quadruplée pour le mode AntiAliasing puis fait la moyenne pour chaque bloc de 4 pixels
void ComputeAllAA(vector<Sphere> &spheres, vector<Cube> &cubes, int width, int height, Vector3f* pixels) {
	vector<Vector3f> pixelsSuperSampled(width*height*4);
	Vector3f* pixels4x = pixelsSuperSampled.data();
	ComputeAll(spheres, cubes, width*2, height*2, pixels4x);
	for(int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			
			pixels[j + width*i] = (pixels4x[2*j + 2*width*(2*i)]
								+ pixels4x[2*j+1 + 2*width*(2*i)]
								+ pixels4x[2*j + 2*width*(2*i+1)]
								+ pixels4x[2*j+1 + 2*width*(2*i+1)])
								/ 4;
		}
	}
}

int main(int argc, char** args) {

	int width = 1280;
	int height = 720;

	loadResolution("parameters/resolution.txt", width, height);

	bool benchmark = false; //Mode benchmark
	bool antiAliasing = false; //Antialiasing
	bool imageMode = false; //Calculer seulement une image et la sauvegarder

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (!strcmp(args[i], "-bm")) {
				benchmark = true;
			}
			else if (!strcmp(args[i], "-aa")) {
				antiAliasing = true;
			}
			else if (!strcmp(args[i], "-i")) {
				imageMode = true;
			}
		}
	}

	//Initialisation de SDL2
	SDL_Window* window = NULL;
	SDL_Renderer* sdlRenderer = NULL;
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "Error initializing SDL: " << SDL_GetError() << endl;
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
        width, height, SDL_WINDOW_SHOWN); //Création de la fenêtre vidéo
	if (window == NULL) { //Vérifier si la fenêtre a bien été créée
        printf("Failed to create display with dimensions %dx%d : %s\n", width, height, SDL_GetError());
        SDL_Quit();
        return 1;
    }



	sdlRenderer = SDL_CreateRenderer(window, -1, 0); //Création d'un renderer

	if (sdlRenderer == NULL) {
		printf("Failed to create renderer with dimensions %dx%d : %s\n", width, height, SDL_GetError());
        SDL_Quit();
		return 1;
	}

	SDL_Texture* sdlTexture = SDL_CreateTexture( sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);



	vector<Sphere> spheres; 	
	addSpheresFromFile("parameters/spheres.csv", spheres); //Lecture du fichier csv pour ajouter les spheres

	vector<Cube> cubes;

	vector<Vector3f> pixels(width*height); //Cration d'un tableau pour stocker les pixels qui seront affichés



	chrono::steady_clock::time_point start, mid, mmid, end;

	chrono::duration<double> t = (chrono::duration<double>)0;

	if (imageMode) {
		start = chrono::steady_clock::now();

		if (!antiAliasing) {
				ComputeAll(spheres, cubes, width, height, pixels.data());
			}
			else {
				ComputeAllAA(spheres, cubes, width, height, pixels.data());
			}
		mid = chrono::steady_clock::now();
		saveImage("results/image.ppm", width, height, pixels.data());
		
		end = chrono::steady_clock::now();
		
		chrono::duration<double> global_elaps = end - start;
		chrono::duration<double> calc_elaps = mid - start;
		chrono::duration<double> save_elaps = end - mid;

		std::cout << "Total : "<< global_elaps.count()*1000 << "ms" << endl;
		std::cout << "Calcul : "<< calc_elaps.count()*1000 << "ms" << endl;
		std::cout << "Ecriture : "<< save_elaps.count()*1000 << "ms" << endl;
	}
	else if (benchmark) { //Mode benchmark

		chrono::steady_clock::time_point global_start = chrono::steady_clock::now();
		int frames = 50;
		for (int i = 0; i < frames; i++) {
			start = chrono::steady_clock::now();

			if (!antiAliasing) {
				ComputeAll(spheres, cubes, width, height, pixels.data());
			}
			else {
				ComputeAllAA(spheres, cubes, width, height, pixels.data());
			}
			renderAll(pixels.data(), width, height, sdlRenderer, sdlTexture);
			SDL_RenderPresent(sdlRenderer);
			
			end = chrono::steady_clock::now();
			
			chrono::duration<double> elaps = end - start;
			t += elaps;

			//Mouvement des sphères
			spheres[1].center(0)=1*sin(t.count()*5);
			spheres[2].center(0)=-10*sin(t.count()*3);
			spheres[2].center(2)=-20+10*cos(t.count()*3);
		}
		chrono::steady_clock::time_point global_end = chrono::steady_clock::now();
		chrono::duration<double> global_elaps = global_end - global_start;
		float fps = frames/global_elaps.count();

		std::cout << omp_get_max_threads() << ","
				<< global_elaps.count() << ","
				<< fps << endl;
	}
		
	else {
		while(1) { //Mode normal

			start = chrono::steady_clock::now();

			if (!antiAliasing) {
				ComputeAll(spheres, cubes, width, height, pixels.data());
			}
			else {
				ComputeAllAA(spheres, cubes, width, height, pixels.data());
			}

			mid = chrono::steady_clock::now();

			renderAll(pixels.data(), width, height, sdlRenderer, sdlTexture); //Affichage sur la texture des données du tableau

			mmid = chrono::steady_clock::now();

			SDL_RenderPresent(sdlRenderer);//Mise à jour de l'écran

			end = chrono::steady_clock::now();



			chrono::duration<double> global_elaps = end - start;
			chrono::duration<double> calc_elaps = mid - start;
			chrono::duration<double> render_elaps = end - mid;
			t+= global_elaps;



			//Mouvement des sphères
			spheres[1].center(0)=1*sin(t.count()*5);
			spheres[2].center(0)=-10*sin(t.count()*2);
			spheres[2].center(2)=-20+10*cos(t.count()*2);



			std::cout <<"Compute time(ms) " << calc_elaps.count()*1000
					<<  ",  Render time(ms) " << render_elaps.count()*1000
					<< ",  Total time(ms) " << global_elaps.count()*1000
					<< ", FPS " << 1/global_elaps.count() << "\r"
					<< std::flush;
			
		}
	}


	//Arrêt de SDL
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}