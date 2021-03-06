#include <cstdio>

#include <vector>

#include "Eigen/Dense"

#include "shapes.h"

#include "ray.h"


using namespace Eigen;
using namespace std;

float mix(const float &a, const float &b, const float &mix) { //Pour reflection de Fresnel
    return b * mix + a * (1 - mix); 
} 


Vector3f trace_sphere(const Vector3f &rayorig, const Vector3f &raydir, vector<Sphere> &spheres, const int &depth) { 
    
    float tnear = INFINITY; 
    const Sphere* sphere = NULL; //La sphère de collision de ce rayon : on cherche à déterminer si elle existe, et si oui quels effets elle a sur le rayon.

    //Pour chaque sphere, on regarde s'il y a intersection avec le rayon tiré
    for (unsigned i = 0; i < spheres.size(); ++i) {
        float d0 = INFINITY, d1 = INFINITY; //Distances au point de collision initialisées à l'infini
        if (spheres[i].intersect(rayorig, raydir, d0, d1)) { //Si collision, détermination du point de collision
            if (d0 < 0) d0 = d1; 
            if (d0 < tnear) { 
                tnear = d0; 
                sphere = &spheres[i]; 
            } 
        } 
    } 


    //Si il n'y a pas de collision, renvoyer la couleur de fond
    if (!sphere) {
        return Vector3f(2,2,2); 
    }

    
    Vector3f surfaceColor = Vector3f(0,0,0); // Stockage pour la surface de la couleur qu'on touche
    Vector3f phit = rayorig + raydir * tnear; //Point d'intersection
    Vector3f nhit = phit - sphere->center; //Vecteur normal au point d'intersection
    nhit.normalize();

    // If the normal and the view direction are not opposite to each other
    // reverse the normal direction. That also means we are inside the sphere so set
    // the inside bool to true. Finally reverse the sign of IdotN which we want
    // positive.
    //Détermination du fait qu'on soit ou non à l'intérieur de la sphere. Cela a des influences pour la réflexion.
    
    float bias = 1e-4; //Création d'une surface autour du point pour détecter s'il y a impact.
    bool inside = false; 
    if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true; //On regarde si le sens du regard et de la normale sont les mêmes. Si oui, on est dans la sphere.

    
    //Si la sphere est transparente ou réfléchissante
    if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) { 
        
        float facingratio = -raydir.dot(nhit); //"Ratio" de vision
        
        //Effet Fresnel
        float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1); 

        //Calcul de la rélfexion
        Vector3f refldir = raydir - nhit * 2 * raydir.dot(nhit); 
        refldir.normalize(); 
        
        //Puisqu'il y a réflexion, on tire un nouveau rayon.
        Vector3f reflection = trace_sphere(phit + nhit * bias, refldir, spheres, depth + 1); 
        Vector3f refraction = Vector3f(0,0,0); 

        // Calcul de la réfraction (=> transmission si transparence)
        if (sphere->transparency > 0) { 
            float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface? 
            float cosi = -nhit.dot(raydir); 
            float k = 1 - eta * eta * (1 - cosi * cosi); 
            Vector3f refrdir = raydir * eta + nhit * (eta *  cosi - sqrt(k)); 
            refrdir.normalize(); 
            //Puisqu'il y a un nouveau rayon, on tire de nouveau
            refraction = trace_sphere(phit - nhit * bias, refrdir, spheres, depth + 1); 
            //printf("Transparence nulle. Couleurs de la sphere : %f, %f, %f\n", sphere->surfaceColor[0], sphere->surfaceColor[1], sphere->surfaceColor[2]);
        } 

        //On ajoute à chaque itération les couleurs appliquées au point après tous les effets de réflexion et réfraction.
        surfaceColor = (reflection*fresneleffect + (refraction * (1 - fresneleffect) * sphere->transparency).cwiseProduct(sphere->surfaceColor));  
    } 
    else { 
        //Si ni transparence ni réflexion, c'est un objet diffusant, pas besoin de continuer le raytracing : il renvoie dans toutes les directions. On arrête la récursion.
        //Possibilité aussi d'avoir dépassé l'indice maximal de récursion
        //On explore alors toutes les sphères à la recherche d'une source de lumière pour savoir si le point est éclairé  : s'il y a un chemin direct entre l'objet et la source.
        
        for (unsigned i = 0; i < spheres.size(); ++i) { 
            if (spheres[i].emissionColor(0) > 0) { 
                //Si source de lumière :
                //Vector3f transmission = Vector3f(1, 1, 1); 
                float transmission = 1.0; //Transmission de la couleur par défaut de 1. Elle sera mise à 0 s'il y a un obstacle.
                Vector3f lightDirection = spheres[i].center - phit; //On détermine la direction entre la source et le point d'impact actuel
                lightDirection.normalize(); 

                //On vérifie maintenant s'il y a un objet entre la lumière et le point d'impact actuel.
                for (unsigned j = 0; j < spheres.size(); ++j) { 
                    if (i != j) { 
                        float d0, d1; 
                        if (spheres[j].intersect(phit + nhit * bias, lightDirection, d0, d1)) { //S'il y a bien un objet, il cache la lumière : la transmission est nulle.
                            //transmission = Vector3f(0, 0, 0); 
                            transmission = 0.0f;
                            break; 
                        } 
                    } 
                } 
                //Ajout du résultat à surfaceColor
                //surfaceColor += ((sphere->surfaceColor.cwiseProduct(transmission)).cwiseProduct(spheres[i].emissionColor)) * std::max(float(0), nhit.dot(lightDirection)); 
                surfaceColor += (((sphere->surfaceColor*transmission) * std::max(float(0), nhit.dot(lightDirection))).cwiseProduct(spheres[i].emissionColor)); 
                
                
                if(sphere->surfaceColor[0]<0.96 && sphere->surfaceColor[0]>0.94){
                    printf("Couleurs de la sphere : %f, %f, %f ; ", sphere->surfaceColor[0], sphere->surfaceColor[1], sphere->surfaceColor[2]);
                    //printf("Indice de la transparence : %f", sphere->transparency);
                    printf("Surface color : %f %f %f ; ", surfaceColor[0], surfaceColor[1], surfaceColor[2]);
                    printf("Produit scalaire : %f \n", nhit.dot(lightDirection));
                }
                
                //surfaceColor += sphere->surfaceColor*transmission* std::max(float(0), nhit.dot(lightDirection)) ; 
                //Produit terme à terme entre la couleur de la sphère et la valeur de transmission (1 ou 0), en produit terme à terme avec la couleur de la lumière, le tout multiplié par le scalaire entre la direction et la normale.
            }
            
        } 
    } 
 
    return surfaceColor + sphere->emissionColor; 
} 


Vector3f trace_cube(const Vector3f &rayorig, const Vector3f &raydir, const std::vector<Cube> &cubes, const int &depth){ //Non implémenté
    return Vector3f(0, 0, 0);
}