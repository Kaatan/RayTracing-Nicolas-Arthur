#include <cstdio>

#include <vector>

#include "Eigen/Dense"

#include "shapes.h"

#include "ray.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif
#ifndef INFINITY
#define INFINITY 1e8
#endif

#ifndef MAX_RAY_DEPTH
#define MAX_RAY_DEPTH 5
#endif

using namespace Eigen;
using namespace std;

float mix(const float &a, const float &b, const float &mix) //Pour reflection de Fresnel
{ 
    return b * mix + a * (1 - mix); 
} 


Vector3f trace_sphere( 
    const Vector3f &rayorig, 
    const Vector3f &raydir, 
    vector<Sphere> &spheres,
    const int &depth) 
{ 
    //if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
    float tnear = INFINITY; 
    const Sphere* sphere = NULL; //LA sphère de collision de ce rayon, qu'on cherche à déterminer si elle existe. 

    // find intersection of this ray with the sphere in the scene
    for (unsigned i = 0; i < spheres.size(); ++i) { //for each sphere
        float d0 = INFINITY, d1 = INFINITY; //distances au point de collision
        if (spheres[i].intersect(rayorig, raydir, d0, d1)) { //Si collision
            if (d0 < 0) d0 = d1; 
            if (d0 < tnear) { 
                tnear = d0; 
                sphere = &spheres[i]; 
            } 
        } 
    } 


    // if there's no intersection return black or background color
    if (!sphere) {
        return Vector3f(2,2,2); 
    }

    
    Vector3f surfaceColor = Vector3f(0,0,0); // color of the ray/surfaceof the object intersected by the ray. Final value returned by the function. 
    Vector3f phit = rayorig + raydir * tnear; // point of intersection 
    Vector3f nhit = phit - sphere->center; // normal at the intersection point 
    nhit.normalize(); // normalize normal direction 
    // If the normal and the view direction are not opposite to each other
    // reverse the normal direction. That also means we are inside the sphere so set
    // the inside bool to true. Finally reverse the sign of IdotN which we want
    // positive.

    float bias = 1e-4; // add some bias to the point from which we will be tracing 
    bool inside = false; 
    if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true; 

    

    if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) { 
        
        float facingratio = -raydir.dot(nhit); //"Ratio" de vision
        
        // change the mix value to tweak the effect
        float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1); 
        // compute reflection direction (not need to normalize because all vectors
        // are already normalized)
        Vector3f refldir = raydir - nhit * 2 * raydir.dot(nhit); 
        refldir.normalize(); 
        Vector3f reflection = trace_sphere(phit + nhit * bias, refldir, spheres, depth + 1); 
        Vector3f refraction = Vector3f(0,0,0); 
        // if the sphere is also transparent compute refraction ray (transmission)
        if (sphere->transparency) { 
            float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface? 
            float cosi = -nhit.dot(raydir); 
            float k = 1 - eta * eta * (1 - cosi * cosi); 
            Vector3f refrdir = raydir * eta + nhit * (eta *  cosi - sqrt(k)); 
            refrdir.normalize(); 
            refraction = trace_sphere(phit - nhit * bias, refrdir, spheres, depth + 1); 
        } 
        // the result is a mix of reflection and refraction (if the sphere is transparent)
        surfaceColor = ( 
            reflection * fresneleffect + 
            (refraction * (1 - fresneleffect) * sphere->transparency).cwiseProduct(sphere->surfaceColor)); //On ajoute à chaque itération les couleurs appliquées au point. 
    } 
    else { 
        // it's a diffuse object, no need to raytrace any further
        for (unsigned i = 0; i < spheres.size(); ++i) { 
            if (spheres[i].emissionColor(0) > 0) { 
                // this is a light source
                Vector3f transmission = Vector3f(1, 1, 1); 
                Vector3f lightDirection = spheres[i].center - phit; 
                lightDirection.normalize(); 

                for (unsigned j = 0; j < spheres.size(); ++j) { 
                    if (i != j) { 
                        float d0, d1; 
                        if (spheres[j].intersect(phit + nhit * bias, lightDirection, d0, d1)) { 
                            transmission = Vector3f(0, 0, 0); 
                            break; 
                        } 
                    } 
                } 
                surfaceColor += (sphere->surfaceColor.cwiseProduct(transmission) * std::max(float(0), nhit.dot(lightDirection))).cwiseProduct(spheres[i].emissionColor); 
            } 
        } 
    } 
 
    return surfaceColor + sphere->emissionColor; 
} 


Vector3f trace_cube(const Vector3f &rayorig, const Vector3f &raydir, const std::vector<Cube> &cubes, const int &depth){
    return Vector3f(0, 0, 0);
}