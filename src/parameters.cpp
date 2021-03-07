#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>

#include "Eigen/Dense"

#include "parameters.h"

using namespace Eigen;
using namespace std;

void loadResolution(string fileName, int &width, int &height){
    ifstream file(fileName);
    file >> width;
    file >> height;
    file.close();
}

void saveImage(string fileName, int width, int height, Vector3f* pixels) {
    ofstream ofs(fileName, std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < width * height; ++i) {
        ofs << (unsigned char)(pixels[i](0) * 255) <<
               (unsigned char)(pixels[i](1) * 255) <<
               (unsigned char)(pixels[i](2) * 255);
    }
    ofs.close();
}