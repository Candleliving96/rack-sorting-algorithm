#ifndef SOURCE_RACK_H
#define SOURCE_RACK_H

#include "Destination_rack.h"
using namespace std;

class Destination_rack;

class Source_rack {
public:
    //constructor
    Source_rack();
    string name;
    int num_samples;
};

#endif