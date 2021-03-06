#ifndef SHAPES_H
#define SHAPES_H

#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>

#include "Eigen/Dense"

#include "shapes.h"

using namespace Eigen;
using namespace std;

class Sphere {
public:
    Vector3f center;                        //Position de la sphère
    float radius, radius2;                  //Rayon de la sphère et carré du rayon
    Vector3f surfaceColor, emissionColor;   //Couleur et couleur d'émission
    float transparency, reflection;         //Transparence et réflexivité
    Sphere(const Vector3f &c, const float &r, const Vector3f &sc, const float &refl,
            const float &transp, const Vector3f &ec);

    //Renvoie true si la sphère est touchée par le rayon mis en argument
    bool intersect(const Vector3f &rayorig, const Vector3f &raydir, float &d0, float &d1) const;
};

class Cube {
public:
    Vector3f center, direction;               //Position du cube
    float side;                               //Côté du cube
    Vector3f surfaceColor, emissionColor;     //Couleur et couleur d'émission
    float transparency, reflection;           //Transparence et réflexivité

    //Renvoie true si le cube est touchée par le rayon mis en argument (non implémenté)
    bool intersect(const Vector3f &rayorig, const Vector3f &raydir, float &d0, float &d1);
};

void addSphereFromLine(vector<string> elem, vector<Sphere> &spheres);
void addSpheresFromFile(string fileName, vector<Sphere> &spheres);

#endif
