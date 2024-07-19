#ifndef BATCH_H
#define BATCH_H

#include <vector>
#include "destination_rack.h"
#include "source_rack.h"
using namespace std;

int const SOURCE_SPOTS = 96;

class Source_rack;
class Destination_rack;

class Batch {
public:
    Batch(int number_destinations, bool remainder);
    ~Batch();

    vector<Source_rack> sources;
    vector<Destination_rack> destinations;

    int retry1;
    int retry2;
    int retry3;
    vector<int> sample_nums_to_add;
    int last_spot;
    int num_sources;
    int num_destinations;
    int add_this;
    int delete_this;
    bool is_remainder;

    void add_largest(array<int, SOURCE_SPOTS>* sample_frequencies_p, bool* switch_19);
    void add_frequencies(array<int, SOURCE_SPOTS>* sample_frequencies_p, vector<Source_rack>* all_sources_p);
    void add_least(array<int, SOURCE_SPOTS>* sample_frequencies_p);
    void calculate_last_spot();
    void decrease_total(bool* switch_decrementing, array<int, SOURCE_SPOTS>* sample_frequencies_p);
    void decrement_highest(int* new_highest, bool* switch_19, array<int, SOURCE_SPOTS>* sample_frequencies_p);

};

#endif
