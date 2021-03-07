#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cstdio>
#include <iostream>
#include <fstream>

using namespace std;

void loadResolution(string fileName, int &width, int &height);

void loadRecursionDepth(string fileName, int& depth);

#endif