#include <iostream>
#include <string>
#include <vector>
#include <array>
#include "batch.h"
#include "Source_rack.h"
#include "Destination_rack.h"
using namespace std;

Batch::Batch(int number_sources, bool remainder) {
	retry1 = 0;
	retry2 = 0;
	retry3 = 0;
	last_spot = 0;
	num_sources = number_sources;
	is_remainder = remainder;
	if (is_remainder) {
		num_destinations = 1;
	}
	else {
		num_destinations = 20 - number_sources;
	}
	add_this = -1;
	delete_this = -1;
}


void Batch::add_largest(array<int, SOURCE_SPOTS>* sample_frequencies_p, bool* switch_19) {
	for (int i = SOURCE_SPOTS - 1; i >= 0; i--) {
		if (sample_frequencies_p->at(i) != 0) {
			//return to switch to 19 if highest is less than 192/3
			if (num_sources == 18 && (i + 1 < 192 / 3)) {
				*switch_19 = true;
			}
			sample_nums_to_add.push_back(i + 1);
			sample_frequencies_p->at(i)--;
			break;
		}
	}
}

void Batch::add_frequencies(array<int, SOURCE_SPOTS>* sample_frequencies_p, vector<Source_rack>* all_sources_p) {
	//calculate how many 1's, 2's, etc to used based on frequencies
	//i is the sample number
	for (int i = 0; i < SOURCE_SPOTS; i++) {
		if (sample_frequencies_p->at(i) == 0) {
			continue;
		}
		if (sample_frequencies_p->at(i) != 0) { //delete after
			int num_i_to_add = (sample_frequencies_p->at(i) * num_sources) / all_sources_p->size();
			if (num_i_to_add > sample_frequencies_p->at(i)) { //check that there are enough to add
				num_i_to_add = sample_frequencies_p->at(i);
			}
			//j is the number of i's to add
			for (int j = 0; j < num_i_to_add; j++) {
				//k is the index of the sample_nums_to_add array
				sample_nums_to_add.push_back(i + 1);
				sample_frequencies_p->at(i)--;
			}
		}
	}
}
void Batch::add_least(array<int, SOURCE_SPOTS>* sample_frequencies_p) {
	int num_sources_left = num_sources - sample_nums_to_add.size();
	//use all but 1 of the leftover sources spots for the lowest sample nums
	//j is the sample number to add
	for (int j = 0; j < SOURCE_SPOTS; j++) {
		//k is the index of the sample_nums_to_add array
		for (int k = 1; k < sample_nums_to_add.size(); k++) {
			if (sample_nums_to_add.at(k) > j + 1) {
				while (num_sources_left > 1) {
					if (sample_frequencies_p->at(j) > num_sources_left) {
						sample_nums_to_add.insert(sample_nums_to_add.begin() + k, j + 1);
						sample_frequencies_p->at(j)--;
						num_sources_left--;
					}
				}
			}
		}
	}
}

void Batch::calculate_last_spot() {
	int total = 0;
	for (int i = 0; i < sample_nums_to_add.size(); i++) {
		total += sample_nums_to_add.at(i);
	}
	last_spot = (num_destinations * 96) - total;
}

void Batch::decrease_total(bool* switch_decrementing, array<int, SOURCE_SPOTS>* sample_frequencies_p) {
    bool reached_end = false;
    *switch_decrementing = false;

    vector<int> copy_sample_nums_to_add;
    for (int i = 0; i < sample_nums_to_add.size(); i++) {
        copy_sample_nums_to_add.push_back(sample_nums_to_add.at(i));
    }

    int delete_this_index = copy_sample_nums_to_add.size() - 1;
    for (int j = 0; j < retry1; j++) {
        for (int i = delete_this_index; i > 0; i--) {
            if (i == 1) {
                reached_end = true;
                break;
            }
            else {
                delete_this_index = i;
                delete_this = copy_sample_nums_to_add.at(i);
                break;
            }
        }
    }

    if (reached_end == true) {
        delete_this_index = copy_sample_nums_to_add.size() - 1;
        delete_this = copy_sample_nums_to_add.at(copy_sample_nums_to_add.size() - 1);
        add_this = copy_sample_nums_to_add.at(0);
        while (add_this < SOURCE_SPOTS - 1 && sample_frequencies_p->at(add_this - 1) == 0) {
            add_this++;
        }
        if (add_this >= SOURCE_SPOTS - 1) {
            cout << "ERROR: check batch.cpp line 125" << endl;
            exit(EXIT_FAILURE);
        }
        if (add_this == delete_this) {
            *switch_decrementing = true;
            return;
        }
    }

    if (delete_this_index = copy_sample_nums_to_add.size() - 1) {
        delete_this = copy_sample_nums_to_add.at(copy_sample_nums_to_add.size() - 1);
    }

    add_this = delete_this - 1;

    for (int i = 0; i < retry2 - 1; i++) {
        add_this--;
    }
    if (sample_frequencies_p->at(add_this - 1) == 0) {
        while (add_this < SOURCE_SPOTS - 1 && sample_frequencies_p->at(add_this - 1) == 0) {
            add_this--;
        }
    }

    if (add_this < 1) {
        retry1++;
        retry2 = 0;
        return;
    }

    int add_this_index = -1;
    for (int i = 1; i < copy_sample_nums_to_add.size(); i++) {
        if (copy_sample_nums_to_add.at(i) == add_this && copy_sample_nums_to_add.at(i) < copy_sample_nums_to_add.at(i + 1)) {
            add_this_index = i;
            break;
        }
    }

    if (add_this_index == -1) {
        cout << "ERROR: check batch.cpp line 164" << endl;
        exit(EXIT_FAILURE);
    }

    copy_sample_nums_to_add.erase(copy_sample_nums_to_add.begin() + delete_this_index);
    copy_sample_nums_to_add.insert(copy_sample_nums_to_add.begin() + add_this_index, add_this);

    int total = 0;
    for (int i = 0; i < copy_sample_nums_to_add.size(); i++) {
        total += copy_sample_nums_to_add.at(i);
    }
    last_spot = (num_destinations * 96) - total;
}

void Batch::decrement_highest(int* new_highest, bool* switch_19, array<int, SOURCE_SPOTS>* sample_frequencies_p) {
    vector<int> copy_sample_nums_to_add;
    for (int i = 0; i < sample_nums_to_add.size(); i++) {
        copy_sample_nums_to_add.push_back(sample_nums_to_add.at(i));
    }

    for (int i = 0; i < retry3; i++) {
        if (copy_sample_nums_to_add.at(0) == 1 && num_destinations == 2) {
            *switch_19 = true;
            return;
        }
        else if (copy_sample_nums_to_add.at(0) == 1 && num_destinations == 1) {
            cout << "ERROR: check line 523" << endl;
            exit(EXIT_FAILURE);
        }
        (copy_sample_nums_to_add.at(0))--;
    }

    while (sample_frequencies_p->at(copy_sample_nums_to_add.at(0) - 1) == 0) {
        if (copy_sample_nums_to_add.at(0) == 1) {
            *switch_19 = true;
            return; //FIGURE OUT HERE
        }
        (copy_sample_nums_to_add.at(0))--;
    }

    if (copy_sample_nums_to_add.at(0) == 1) {
        *switch_19 = true; //change to 19/1!!
        return;
    }

    //update new_highest and last_spot
    *new_highest = copy_sample_nums_to_add.at(0);
    
    int total = 0;
    for (int i = 0; i < copy_sample_nums_to_add.size(); i++) {
        total += copy_sample_nums_to_add.at(i);
    }
    last_spot = (num_destinations * 96) - total;
}

void check(array<int, SOURCE_SPOTS>* sample_frequencies_p, bool* switch_remainder) {
    //check if there are enough samples to not have to switch to remainder
    int copy_sample_frequencies[SOURCE_SPOTS];
    for (int i = 0; i < SOURCE_SPOTS; i++) {
        copy_sample_frequencies[i] = sample_frequencies_p->at(i);
    }

    int total = 0;
    int sources_left = 19;
    for (int i = SOURCE_SPOTS - 1; i >= 0; i--) {
        while (sources_left > 0) {
            if (copy_sample_frequencies[i] == 0) {
                break;
            }
            total += i;
            sources_left--;
            copy_sample_frequencies[i]--;
        }
    }

    //if not enough, switch to remainder now
    if (total < SOURCE_SPOTS) {
        *switch_remainder = true;
    }
}

//destructor
Batch::~Batch() {
	//delete sources;
	//delete destinations;
}