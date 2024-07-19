#ifndef DESTINATION_RACK_H
#define DESTINATION_RACK_H

#include <string>
#include "Source_rack.h"
using namespace std;
class Source_rack;

class Destination_rack {
public:
    //constructor
    Destination_rack();

    string name;
    int num_spots_filled;
};

#endif
