#ifndef PROGRAM_H
#define PROGRAM_H

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include "Source_rack.h"
#include "Destination_rack.h"
#include "Batch.h"

using namespace std;

int const SOURCE_SPOTS = 96;
class Program {
	int num_sources;
	vector<Source_rack> all_sources;
	vector<Batch> all_batches;
	array<int, SOURCE_SPOTS> sample_frequencies;
	bool switch_remainder;
	
};

#endif