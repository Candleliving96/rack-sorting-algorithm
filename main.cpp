#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include "Source_rack.h"
#include "Destination_rack.h"
#include "Batch.h"
using namespace std;

vector<Source_rack> read_racks(string filename);
void distribute_racks(int num_sources, vector<Source_rack> *all_sources_p, vector<Batch> *all_batches_p, array<int, SOURCE_SPOTS>* sample_frequencies_p);
void distribute_remainder(vector<Source_rack>* all_sources_p, vector<Batch>* all_batches_p, array<int, SOURCE_SPOTS>* sample_frequencies_p);
void execute_retry2(bool* switch_remainder, array<int, SOURCE_SPOTS>* sample_frequencies_p, Batch* current_batch);
//void decrement_highest(int *new_highest, bool *switch_19, Batch *current_batch, array<int, SOURCE_SPOTS>* sample_frequencies_p);

void check(array<int, SOURCE_SPOTS>* sample_frequencies_p, bool* switch_remainder);


int main(int argc, char* argv[]) {
    
    string file_name;
    cout << "This is the Rack Distribution Program, please enter the name of a data file (must include .txt extension at end of name)" << endl;
    cin >> file_name;

    //Program my_program;

    vector<Source_rack> all_sources = read_racks(file_name);
    vector<Source_rack>* all_sources_p = &all_sources;
    vector<Batch> all_batches;
    vector<Batch>* all_batches_p = &all_batches;

    array<int, SOURCE_SPOTS> sample_frequencies; //index 0 is the number of 1's - index n is the number of n + 1's
    for (int i = 0; i < SOURCE_SPOTS; i++) {
        sample_frequencies.at(i) = 0;
    }

    //get how many 1's, 2's, 3's, etc there are
    for (int i = 1; i < SOURCE_SPOTS + 1; i++) {
        for (int j = 0; j < all_sources_p->size(); j++) {
            if (all_sources_p->at(j).num_samples == i) {
                sample_frequencies.at(i - 1)++;
            }
        }
    }
    array<int, SOURCE_SPOTS>* sample_frequencies_p = &sample_frequencies;

    distribute_racks(18, all_sources_p, all_batches_p, sample_frequencies_p);
    distribute_racks(19, all_sources_p, all_batches_p, sample_frequencies_p);
    distribute_remainder(all_sources_p, all_batches_p, sample_frequencies_p);

    string overview;
    cout << "The program has created a solution, would you like to see to see a reader-friendly overview before exporting as a text file? (y/n)" << endl;
    cin >> overview;

    if (overview == "y" || overview == "yes") {
        cout << "Number of batches is: " << all_batches.size() << endl;
        for (int i = 0; i < all_batches.size(); i++) {
            cout << "-- Batch number " << i + 1 << " --" << endl;
            cout << "Number of sources in this batch: " << all_batches.at(i).sources.size() << endl;

            int num_destinations;
            if (all_batches.at(i).sources.size() == 18 || all_batches.at(i).sources.size() == 19) {
                num_destinations = 20 - all_batches.at(i).sources.size();
            }
            else {
                num_destinations = 1;
            }
            cout << "Number of destinations in this batch: " << num_destinations << endl; // FIX LAST BATCH

            int total_spots_filled = 0;
            for (int k = 0; k < all_batches.at(i).sources.size(); k++) {
                total_spots_filled += all_batches.at(i).sources.at(k).num_samples;
            }
            cout << "Number of spots filled in destination racks: " << total_spots_filled << endl;

            /*cout << "Source racks: " << endl;
            for (int j = 0; j < all_batches.at(i).sources.size(); j++) {
                cout << "Rack ID: " << all_batches.at(i).sources.at(j).name << ", sample number: ";
                cout << all_batches.at(i).sources.at(j).num_samples << endl;
            }*/
        }
    }
    

    cout << endl << "A text file version compatible with excel will be exported, please enter the name you would like to save the file as:" << endl;
    string output_name;
    cin >> output_name;
    string test = output_name.substr(output_name.size() - 4, 4);
    if (test != ".txt") {
        output_name += ".txt";
    }

    ofstream outfile;
    outfile.open(output_name);
    if (outfile.fail()) {
        cout << "Error creating file. Please check that the inputted file name is valid and re-run program" << endl;
    }

    string output = "Rack ID,Sample Number,Batch Number,Number of Samples In Batch\n";

    for (int i = 0; i < all_batches.size(); i++) {
        for (int j = 0; j < all_batches.at(i).sources.size(); j++) {
            //add name/ID to output
            output += (all_batches.at(i).sources.at(j).name);
            output += ",";

            //add number of samples in rack to output
            output += to_string(all_batches.at(i).sources.at(j).num_samples);
            output += ",";

            //add batch number to output
            output += to_string(i + 1);
            output += ",";

            //add number of samples in batch
            int total_spots_filled = 0;
            for (int k = 0; k < all_batches.at(i).sources.size(); k++) {
                total_spots_filled += all_batches.at(i).sources.at(k).num_samples;
            }
            output += to_string(total_spots_filled);
            output += "\n";
        }
        output += "\n";
    }

    outfile << output << endl;
    cout << endl << "Your text file has been created with the name " << output_name << endl;

    outfile.close();
    return 0;
}



void execute_retry2(bool* switch_remainder, array<int, SOURCE_SPOTS>* sample_frequencies_p, Batch *current_batch) { 
    vector<int> copy_sample_nums_to_add;
    for (int i = 0; i < current_batch->sample_nums_to_add.size(); i++) {
        copy_sample_nums_to_add.push_back(current_batch->sample_nums_to_add.at(i));
    }
    
    int delete_this_index = 0;
    bool reached_end = false;
    for (int j = 0; j < current_batch->retry1; j++) {
        for (int i = delete_this_index + 1; i < copy_sample_nums_to_add.size(); i++) {
            if (i == copy_sample_nums_to_add.size() - 1) {
                reached_end = true;
                break;
            }
            else if (copy_sample_nums_to_add.at(i) < copy_sample_nums_to_add.at(i + 1)) {
                delete_this_index = i;
                current_batch->delete_this = copy_sample_nums_to_add.at(i);
                break;
            }
            else if (copy_sample_nums_to_add.at(i) == copy_sample_nums_to_add.at(copy_sample_nums_to_add.size() - 1)) {
                delete_this_index = copy_sample_nums_to_add.size() - 1;
                current_batch->delete_this = copy_sample_nums_to_add.at(i);
                break;
            }
        }
    }

    //case that delete_this has reached the end
    if (reached_end == true) {
        delete_this_index = 1;
        current_batch->delete_this = copy_sample_nums_to_add.at(1);
        current_batch->add_this = copy_sample_nums_to_add.at(copy_sample_nums_to_add.size() - 1) + 1;
        while (current_batch->add_this < SOURCE_SPOTS - 1 && sample_frequencies_p->at(current_batch->add_this - 1) == 0) {
            current_batch->add_this++;
        }
        if (current_batch->add_this >= SOURCE_SPOTS - 1) {
            if (current_batch->num_destinations == 1) {
                *switch_remainder = true;
                return;
            }
            else {
                return;
            }
        }

        for (int i = 0; i < current_batch->sample_nums_to_add.size(); i++) {
            if (current_batch->sample_nums_to_add.at(i) == current_batch->delete_this) {
                current_batch->sample_nums_to_add.erase(current_batch->sample_nums_to_add.begin() + i);
                break;
            }
        }
        //update delete_this in sample_frequencies
        (sample_frequencies_p->at(current_batch->delete_this - 1))++;

        //updating add_this in the actual sample_frequencies array AND sample_nums_to_add array    
        if (current_batch->add_this >= (current_batch->sample_nums_to_add.at(current_batch->sample_nums_to_add.size() - 1))) {
            current_batch->sample_nums_to_add.push_back(current_batch->add_this);
            (sample_frequencies_p->at(current_batch->add_this - 1))--;
        }
        else {
            for (int j = 1; j < current_batch->sample_nums_to_add.size(); j++) {
                if (current_batch->sample_nums_to_add.at(j) == current_batch->add_this && current_batch->sample_nums_to_add.at(j) < current_batch->sample_nums_to_add.at(j + 1)) {
                    (sample_frequencies_p->at(current_batch->add_this - 1))--;
                    current_batch->sample_nums_to_add.insert(current_batch->sample_nums_to_add.begin() + j, current_batch->add_this);
                    break;
                }
            }
        }
        return;
    }

    if (delete_this_index == 0) {
        delete_this_index = 1;
        current_batch->delete_this = copy_sample_nums_to_add.at(1);
    }

    current_batch->add_this = current_batch->delete_this + 1;
    for (int i = 0; i < current_batch->retry2 - 1; i++) {
        current_batch->add_this++;
    }

    if (sample_frequencies_p->at(current_batch->add_this - 1) == 0) {
        while (current_batch->add_this < SOURCE_SPOTS - 1 && sample_frequencies_p->at(current_batch->add_this - 1) == 0) {
            current_batch->add_this++;
        }
    }

    if (current_batch->add_this >= SOURCE_SPOTS - 1) {
        current_batch->retry1++;
        current_batch->retry2 = 0;
        return;
    }

    int add_this_index = -1;
    for (int i = 1; i < copy_sample_nums_to_add.size(); i++) {
        if (current_batch->add_this >= copy_sample_nums_to_add.at(copy_sample_nums_to_add.size() - 1)) {
            add_this_index = copy_sample_nums_to_add.size() - 1;
            break;
        }
        else if (copy_sample_nums_to_add.at(i) == current_batch->add_this && copy_sample_nums_to_add.at(i) < copy_sample_nums_to_add.at(i + 1)) {
            add_this_index = i;
            break;
        }
        else if (current_batch->add_this > copy_sample_nums_to_add.at(i) && current_batch->add_this < copy_sample_nums_to_add.at(i + 1)) {
            add_this_index = i + 1;
            break;
        }
    }

    if (add_this_index == -1) { //means it never went through
        cout << "ERROR: check line 246" << endl;
        exit(EXIT_FAILURE);
    }

    copy_sample_nums_to_add.erase(copy_sample_nums_to_add.begin() + delete_this_index);
    copy_sample_nums_to_add.insert(copy_sample_nums_to_add.begin() + add_this_index, current_batch->add_this);

    int total = 0;
    for (int i = 0; i < copy_sample_nums_to_add.size(); i++) {
        total += copy_sample_nums_to_add.at(i);
    }
    current_batch->last_spot = (current_batch->num_destinations * 96) - total;
}


void distribute_remainder(vector<Source_rack>* all_sources_p, vector<Batch>* all_batches_p, array<int, SOURCE_SPOTS>* sample_frequencies_p) {

    while (all_sources_p->size() >= 19) {
        Batch current_batch(19, true);
        vector<int> sample_nums_to_add;
        int num_sources_left = 19;
        for (int i = SOURCE_SPOTS - 1; i > 0; i--) {
            if (sample_frequencies_p->at(i) != 0) {
                sample_nums_to_add.push_back(i + 1);
                sample_frequencies_p->at(i)--;
                num_sources_left--;
                break;
            }
        }

        //i is the sample number (-1??)
        for (int i = 0; i < SOURCE_SPOTS; i++) {
            if (sample_frequencies_p->at(i) == 0) {
                continue;
            }
            if (sample_frequencies_p->at(i) != 0) { //delete after
                int num_i_to_add = (sample_frequencies_p->at(i) * 19) / all_sources_p->size();
                if (num_i_to_add > sample_frequencies_p->at(i)) { //check that there are enough to add
                    num_i_to_add = sample_frequencies_p->at(i);
                }
                //j is the number of i's to add
                for (int j = 0; j < num_i_to_add; j++) {
                    //k is the index of the sample_nums_to_add array
                    sample_nums_to_add.push_back(i + 1);
                    num_sources_left--;
                    sample_frequencies_p->at(i)--;
                }
            }
        }

        //use all but 1 of the leftover sources spots for the lowest sample nums
        //j is the sample number to add (-1??)
        int j = 0;
        while (num_sources_left > 0) {
            if (j > 95) {
                cout << endl << "ERROR: check line 434" << endl;
                exit(EXIT_FAILURE);
            }
            for (int k = 1; k < sample_nums_to_add.size(); k++) {
                if (sample_nums_to_add.at(k) > j + 1 && sample_frequencies_p->at(j) > 0) {
                    sample_nums_to_add.insert(sample_nums_to_add.begin() + k, j + 1);
                    sample_frequencies_p->at(j)--;
                    num_sources_left--;
                    break;
                }
                else if (sample_nums_to_add.at(k) == sample_nums_to_add.at(sample_nums_to_add.size() - 1) && sample_frequencies_p->at(j) > 0) {
                    sample_nums_to_add.push_back(j + 1);
                    sample_frequencies_p->at(j)--;
                    num_sources_left--;
                    break;
                }
                else if (sample_frequencies_p->at(j) == 0) {
                    j++;
                }
            }
        }
       
        for (int i = 0; i < sample_nums_to_add.size(); i++) {
            for (int j = 0; j < all_sources_p->size(); j++) {
                if (sample_nums_to_add.at(i) == all_sources_p->at(j).num_samples) {
                    current_batch.sources.push_back(all_sources_p->at(j));
                    all_sources_p->erase(all_sources_p->begin() + j);
                    break;
                }
            }
        }
        all_batches_p->push_back(current_batch);

    }

    Batch last_batch(all_sources_p->size(), true);
    for (int i = 0; i < all_sources_p->size(); i++) {
        last_batch.sources.push_back(all_sources_p->at(i));
    }

    for (int i = 0; i < all_sources_p->size(); i++) {
        all_sources_p->erase(all_sources_p->begin());
    }

    all_batches_p->push_back(last_batch);

    for (int i = 0; i < SOURCE_SPOTS; i++) {
        for (int j = 0; j < last_batch.sources.size(); j++) {
            if (i + 1 == last_batch.sources.at(j).num_samples) {
                sample_frequencies_p->at(i)--;
            }
        }
    }

}

void increase_last_spot(Batch* current_batch, array<int, SOURCE_SPOTS>* sample_frequencies_p, bool *switch_decrementing, bool switch_19) {
    current_batch->retry1 = 0;
    current_batch->retry2 = 0;
    while (current_batch->last_spot < 1) {
        current_batch->retry2++;
        current_batch->decrease_total(switch_decrementing, sample_frequencies_p);

        if (*switch_decrementing == true) {
            current_batch->retry3++;
            int new_highest;
            current_batch->decrement_highest(&new_highest, &switch_19, sample_frequencies_p);
            if (switch_19 == true) {
                return;
            }
            if (current_batch->last_spot > 1 && current_batch->last_spot < SOURCE_SPOTS - 1) {
                //put back old highest
                (sample_frequencies_p->at(current_batch->sample_nums_to_add.at(0) - 1))++;
                current_batch->sample_nums_to_add.erase(current_batch->sample_nums_to_add.begin());
                //update new highest
                current_batch->sample_nums_to_add.insert(current_batch->sample_nums_to_add.begin(), new_highest);
                sample_frequencies_p->at(new_highest - 1)++;
            }
        }
    }
}

void distribute_racks(int num_sources, vector<Source_rack>* all_sources_p, vector<Batch>* all_batches_p, array<int, SOURCE_SPOTS>* sample_frequencies_p) {

    int num_sources_left = num_sources;
    int num_destinations = 20 - num_sources;
    bool switch_remainder = false;
    bool switch_19 = false;
    bool switch_decrementing = false;

    int remainder = all_sources_p->size() % num_sources;

    while (all_sources_p->size() > remainder) {
        //create new batch
        Batch current_batch(num_sources, false);

        //check that there are enough samples to not have to go to remainder
        check(sample_frequencies_p, &switch_remainder);
        if (switch_remainder == true) {
            return;
        }
        
        //add the largest available source to the batch's source vector
        current_batch.add_largest(sample_frequencies_p, &switch_19);
        if (switch_19 == true) {
            //put back the largest b/c never used
            sample_frequencies_p->at(current_batch.sample_nums_to_add.at(0) - 1)++;
            return;
        }
        
        //add frequencies
        current_batch.add_frequencies(sample_frequencies_p, all_sources_p);

        //use all but 1 of the leftover sources spots for the lowest sample nums
        current_batch.add_least(sample_frequencies_p);
        //use the last sources spot for the leftover value, adjusting if not available
        //int retry1 = 0;
        //int retry2 = 0;
        //int retry3 = 0;
        //int last_spot;
        int add_this = -1;
        int delete_this = -1;
        
        current_batch.calculate_last_spot();

        if (current_batch.last_spot < 1) {
            increase_last_spot(&current_batch, sample_frequencies_p, &switch_decrementing, &switch_19);
        }
        
        //execute retry2 to decrease last_spot
        while (current_batch.last_spot > SOURCE_SPOTS || sample_frequencies_p->at(current_batch.last_spot - 1) == 0) {
            
            switch_remainder = false;
            current_batch.retry2++;
            execute_retry2(&switch_remainder, sample_frequencies_p, &current_batch);

            if (switch_remainder == true && num_destinations == 1) {
                //switch to remainder
                //update frequencies - put back because never used
                for (int i = 0; i < current_batch.sample_nums_to_add.size(); i++) {
                    (sample_frequencies_p->at(current_batch.sample_nums_to_add.at(i) - 1))++;
                }
                return;
            }

            else if (switch_remainder == true && num_destinations != 1) {
                current_batch.retry3++;
                //switch to decrementing
                int new_highest;
                current_batch.decrement_highest(&new_highest, &switch_19, sample_frequencies_p);
                if (current_batch.last_spot > 1 && current_batch.last_spot < SOURCE_SPOTS - 1) {
                    (sample_frequencies_p->at(current_batch.sample_nums_to_add.at(0) - 1))++;
                    current_batch.sample_nums_to_add.at(0) = new_highest;
                    sample_frequencies_p->at(new_highest - 1)++;
                }
            }
        }

        if (current_batch.delete_this != -1 && current_batch.add_this != -1) {
            //update delete_this in sample_nums_to_add
            for (int i = 0; i < current_batch.sample_nums_to_add.size(); i++) {
                if (current_batch.sample_nums_to_add.at(i) == current_batch.delete_this) {
                    current_batch.sample_nums_to_add.erase(current_batch.sample_nums_to_add.begin() + i);
                    break;
                }
            }
            //update delete_this in sample_frequencies
            (sample_frequencies_p->at(current_batch.delete_this - 1))++;

            //updating add_this in the actual sample_frequencies array AND sample_nums_to_add array    
            if (current_batch.add_this >= (current_batch.sample_nums_to_add.at(current_batch.sample_nums_to_add.size() - 1))) {
                current_batch.sample_nums_to_add.push_back(current_batch.add_this);
                (sample_frequencies_p->at(current_batch.add_this - 1))--;
            }
            else {
                for (int j = 1; j < current_batch.sample_nums_to_add.size(); j++) {
                    if (current_batch.sample_nums_to_add.at(j) == current_batch.add_this && current_batch.sample_nums_to_add.at(j) < current_batch.sample_nums_to_add.at(j + 1)) {
                        (sample_frequencies_p->at(current_batch.add_this - 1))--;
                        current_batch.sample_nums_to_add.insert(current_batch.sample_nums_to_add.begin() + j, current_batch.add_this);
                        break;
                    }
                }
            }
        }

        current_batch.sample_nums_to_add.push_back(current_batch.last_spot);
        sample_frequencies_p->at(current_batch.last_spot - 1)--;
        num_sources_left--;

        for (int i = 0; i < current_batch.sample_nums_to_add.size(); i++) {
            for (int j = 0; j < all_sources_p->size(); j++) {
                if (current_batch.sample_nums_to_add.at(i) == all_sources_p->at(j).num_samples) {
                    current_batch.sources.push_back(all_sources_p->at(j));
                    all_sources_p->erase(all_sources_p->begin() + j);
                    break;
                }
            }
        }
        all_batches_p->push_back(current_batch);
    }
}

vector<Source_rack> read_racks(string filename) {
    ifstream infile;
    infile.open(filename);
    if (infile.fail()) {
        cout << "Error opening file. Check that file name is valid and that the file is located in the same folder as the RackProject.vcxproj program" << endl;
        exit(EXIT_FAILURE);
    }
    vector<Source_rack> all_sources;
    while (!infile.eof()) {
        Source_rack current_source;
        infile >> current_source.name;
        infile >> current_source.num_samples;
        all_sources.push_back(current_source);
    }
    return all_sources;
}