#ifndef RAY_H
#define RAY_H

#include <cstdio>
#include <vector>

#include "Eigen/Dense"

#include "shapes.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif
#ifndef INFINITY
#define INFINITY 1e8
#endif

//Profondeur maximale de récursion
#ifndef MAX_RAY_DEPTH
#define MAX_RAY_DEPTH 3
#endif

using namespace Eigen;
using namespace std;

float mix(const float &a, const float &b, const float &mix);

//Trace un rayon en prenant en compte les sphères placées et renvoie une couleur qui correspondra à un pixel
Vector3f trace_sphere(const Vector3f &rayorig, const Vector3f &raydir, vector<Sphere> &spheres, const int &depth);

//Non implémenté
Vector3f trace_cube(const Vector3f &rayorig, const Vector3f &raydir, const std::vector<Cube> &cubes, const int &depth);

#endif





