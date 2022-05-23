#include<string.h>
#include<iostream>
#include<fstream>
#include<algorithm>
#include<vector>
#include<set>
#include<iterator>
#include<sodium.h>

using namespace std;

namespace util {

void hash_element(unsigned char hash_loc[], const unsigned char* element, size_t element_size, size_t size) {
    crypto_generichash(hash_loc, size,
            element, element_size,
            NULL, 0);
}

int load_file(string *data_loc, string file_loc) {
    string line;
    int i=0;
    ifstream mFile(file_loc);
    if(mFile.is_open()) {
        while(!mFile.eof()) {
            getline(mFile, line);
            data_loc[i]=line;
            i++;
        }
        mFile.close();
    }
    return i;
}

int load_file_vector(vector<string> &data_loc, string file_loc) {
    string line;
    int i=0;
    ifstream mFile(file_loc);
    if(mFile.is_open()) {
        while(!mFile.eof()) {
            getline(mFile, line);
            data_loc.push_back(line);
            i++;
        }
        mFile.close();
    }
    return i;
}

void intersection(string outfilename, string first_file, string second_file, string original_file, int celements, int selements){
    cout << "[LOG] : Doing the intersection" << endl;
    string* first = new string[celements + 1];
    string* second = new string[selements + 1];
    
    vector<string> original;
    string* c = new string[celements + 1];
    
    // Read from first file
    int i1 = load_file(first, first_file);

    int i2 = load_file(second, second_file);

    load_file_vector(original, original_file);

    load_file(c, first_file);

    vector<string> v(i1 + i2);
    vector<string>::iterator it, st;
    
    // Sorting both lists
    sort(first, first + i1);
    sort(second, second + i2);
    
    it = set_intersection(first, first + i1, second, second + i2, v.begin());
    set<string> intersection_set;
    st++;
    for (st = v.begin(); st != it; ++st) {
        // fprintf(file, "%s\n", (*st).c_str());
        intersection_set.insert((*st).c_str());
    }

    FILE *file;
    file = fopen(outfilename.c_str(), "wt");

    int index = 0;
    for (auto &e : original) {
        auto find = intersection_set.find(c[index]);
        if (find != intersection_set.end()) {
            
            string intersect_e = *find;
            if (c[index] == intersect_e && intersect_e != "") {
                fprintf(file, "%s\n", original[index].c_str());
            }
        }
        index++;
    }
    fclose(file);
}
};