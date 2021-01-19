#ifndef SHAPES_H
#define SHAPES_H

#include <cstdio>
#include "Eigen/Dense"

using namespace Eigen;

class Sphere {
public:
    Vector3f center;                           /// position of the sphere
    float radius, radius2;                  /// sphere radius and radius^2
    Vector3f surfaceColor, emissionColor;      /// surface color and emission (light)
    float transparency, reflection;         /// surface transparency and reflectivity
    Sphere(const Vector3f &c, const float &r, const Vector3f &sc, const float &refl,
            const float &transp, const Vector3f &ec);

    bool intersect(const Vector3f &rayorig, const Vector3f &raydir, float &d0, float &d1) const;
};

class Cube {
public:
    Vector3f center, direction;                           /// position of the cube
    float side;
    Vector3f surfaceColor, emissionColor;      /// surface color and emission (light)
    float transparency, reflection;         /// surface transparency and reflectivity

    bool intersect(const Vector3f &rayorig, const Vector3f &raydir, float &d0, float &d1);
};

#endif
