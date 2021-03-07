#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include "Eigen/Dense"

#include "parameters.h"

using namespace Eigen;
using namespace std;

void loadResolution(string fileName, int &width, int &height);

void saveImage(string fileName, int width, int height, Vector3f* pixels);

#endif