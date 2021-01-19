#include <cstdio>
#include "Eigen/Dense"

#include "shapes.h"

using namespace Eigen;


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

bool Cube::intersect(const Vector3f &rayorig, const Vector3f &raydir, float &d0, float &d1) {
    return true;
}