#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>

#include "Eigen/Dense"

#include "shapes.h"
#include "csv_reader.cpp"

using namespace Eigen;
using namespace std;


//Constructeur pour les Spheres
Sphere::Sphere(const Vector3f &c, const float &r, const Vector3f &sc, const float &refl = 0,
        const float &transp = 0, const Vector3f &ec = Vector3f(0,0,0)) {
    center = c;
    radius = r;
    radius2 = r*r;
    surfaceColor = sc;
    emissionColor = ec;
    transparency = transp;
    reflection = refl;
    }


//Renvoie true si la sphère est touchée par le rayon mis en argument
bool Sphere::intersect(const Vector3f &rayorig, const Vector3f &raydir, float &d0, float &d1) const {
    Vector3f l = center - rayorig;
    float tca = l.dot(raydir);
    if (tca < 0) return false;
    float d2 = l.dot(l) - tca * tca;
    if (d2 > radius2) return false;
    float thc = sqrt(radius2 - d2);
    d0 = tca - thc;
    d1 = tca + thc;
    
    return true;
}

//Non implémenté
bool Cube::intersect(const Vector3f &rayorig, const Vector3f &raydir, float &d0, float &d1) {
    return true;
}

//Convertit un vecteur de string en données pour Sphere
void addSphereFromLine(vector<string> elem, vector<Sphere> &spheres) { 
	try {
		float x,y,z; //Position de la sphère
		float R; //Rayon
		float r,g,b; //Couleur
		float reflectivity, transparency;
		float er, eg, eb; //Couleur d'émission

		//Traduction en flottant des données de elem
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

		//Ajout de la sphère correspondante
		spheres.push_back(Sphere(Vector3f( x, y, z), R, Vector3f(r, g, b), reflectivity, transparency, Vector3f(er,eg,eb))); 
	}
	catch(...) {cout << "Incorrect sphere inputs parameters";}
}



void addSpheresFromFile(string fileName, vector<Sphere> &spheres) { //Récupère et ajoute les données du CSV pour les sphères
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