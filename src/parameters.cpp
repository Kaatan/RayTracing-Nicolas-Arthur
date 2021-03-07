#include <cstdio>
#include <iostream>
#include <fstream>

#include "parameters.h"

using namespace std;

void loadResolution(string fileName, int &width, int &height){
    ifstream file(fileName);
    file >> width;
    file >> height;
    file.close();
}