#include<sodium.h>
#include<cstring>
#include<cstdlib>
#include<sstream>
#ifndef UTIL_H
#include"util.h"
#define UTIL_H
#endif
#include"../cuckoofilter-master/src/cuckoofilter.h"

using namespace std;

namespace cuckoo {

unsigned char hash_loc[crypto_core_ristretto255_HASHBYTES];
unsigned char hash64[8];

void server_hash_and_op(string infilename, string outfilename, unsigned char * key, bool do_hash, int filter_size) {
    unsigned char op_value[crypto_core_ristretto255_BYTES];
    unsigned char px[crypto_core_ristretto255_BYTES];
    unsigned char bin[crypto_core_ristretto255_BYTES];
    unsigned char bin64[8];

    ifstream infile(infilename);
    string line;
        
    if (do_hash) {
        cuckoofilter::CuckooFilter<ulong, 16> filter(filter_size);

        int index = 0;
        while (getline(infile, line)) {
            const unsigned char *element = reinterpret_cast<const unsigned char*> (line.c_str());
            util::hash_element(hash_loc, element, strlen((const char*)element), crypto_core_ristretto255_HASHBYTES);
            crypto_core_ristretto255_from_hash(px, hash_loc);

            int i = crypto_scalarmult_ristretto255(op_value, key, px);

            util::hash_element(hash64, op_value, 32, 8);
            char hex[(8 * 2) + 1];
            sodium_bin2hex(hex, sizeof hex, hash64, 8);
            sodium_hex2bin(bin64, 8, (const char*)hex, 16, NULL, NULL, NULL);

            filter.Add(*(ulong*) bin64);
            // filter.Add(*(ulong*) op_value);

            index++;
        }

        cout << "Size of filter:" << filter.Size() << endl;
        cout << "Size in bytes:" << filter.SizeInBytes() << endl;
        cout << "Info:" << filter.Info() << endl;

        FILE *file_out;
        file_out = fopen(outfilename.c_str(), "wt");
        filter.WriteFile(file_out);
        fclose(file_out);
    }
    else {
        FILE *file_out;
        file_out = fopen(outfilename.c_str(), "wt");

        while (getline(infile, line)) {
            const unsigned char *element = reinterpret_cast<const unsigned char*> (line.c_str());
            
            int succ_h2b = sodium_hex2bin(bin, 32, (const char*)element, 64, NULL, NULL, NULL);
            int i = crypto_scalarmult_ristretto255(op_value, key, bin);
            char hex[(crypto_generichash_BYTES * 2) + 1];
            sodium_bin2hex(hex, sizeof hex, op_value, crypto_generichash_BYTES);
            fprintf(file_out, "%s\n", hex);
        }
        fclose(file_out);
    }
}

void client_hash_and_op(string infilename, string outfilename, unsigned char * key) {
    unsigned char op_value[crypto_core_ristretto255_BYTES];
    unsigned char px[crypto_core_ristretto255_BYTES];
    unsigned char bin[crypto_core_ristretto255_BYTES];

    ifstream infile(infilename);

    FILE *file_out;
    file_out = fopen(outfilename.c_str(), "wt");

    string line;

    while (getline(infile, line)) {
        const unsigned char *element = reinterpret_cast<const unsigned char*> (line.c_str());            
        util::hash_element(hash_loc, element, strlen((const char*)element), crypto_core_ristretto255_HASHBYTES);
        crypto_core_ristretto255_from_hash(px, hash_loc);

        int i = crypto_scalarmult_ristretto255(op_value, key, px);
        
        char hex[(crypto_core_ristretto255_BYTES * 2) + 1];
        sodium_bin2hex(hex, sizeof hex, op_value, crypto_core_ristretto255_BYTES);

        fprintf(file_out, "%s\n", hex);
    }
    fclose(file_out);
    infile.close();
}

void read_and_intersect(string filter_file, string elements, string originals, int celements, int filter_size) {
    cuckoofilter::CuckooFilter<ulong, 16> filter(filter_size);

    FILE *filter_in;
    filter_in = fopen("./tmp/b.txt", "rd");
    filter.ReadFile(filter_in);

    string* original = new string[celements + 1];
    util::load_file(original, originals);

    FILE *outfile;
    outfile = fopen("./tmp/intersection.txt", "wt");

    ifstream infile(elements);
    string line;
    unsigned char bin[crypto_core_ristretto255_BYTES];
    unsigned char bin64[8];
    
    int i = 0;
    while (getline(infile, line)) {
        const unsigned char *element = reinterpret_cast<const unsigned char*> (line.c_str());
        int succ_h2b = sodium_hex2bin(bin, 32, (const char*)element, 64, NULL, NULL, NULL);

        util::hash_element(hash64, bin, 32, 8);
        char hex[(8 * 2) + 1];
        sodium_bin2hex(hex, sizeof hex, hash64, 8);
        sodium_hex2bin(bin64, 8, (const char*)hex, 16, NULL, NULL, NULL);

        cuckoofilter::Status status = filter.Contain(*(ulong*)bin64);
        // cuckoofilter::Status status = filter.Contain(*(ulong*)bin);
        if (status == 0) {
            fprintf(outfile, "%s\n", original[i].c_str());
        }
        i++;
    }
    fclose(outfile);
    infile.close();   
}

void invert(string infilename, string outfilename, unsigned char * key) {
    unsigned char bin[crypto_core_ristretto255_BYTES];
    unsigned char bin2[crypto_core_ristretto255_BYTES];
    unsigned char inv[crypto_core_ristretto255_SCALARBYTES];
    
    int i = crypto_core_ristretto255_scalar_invert(inv, key);

    ifstream infile(infilename);

    FILE *file_out;
    file_out = fopen(outfilename.c_str(), "wt");

    string line;

    while (getline(infile, line)) {
        const unsigned char *element = reinterpret_cast<const unsigned char*> (line.c_str());

        int succ_h2b = sodium_hex2bin(bin, 32, (const char*)element, 64, NULL, NULL, NULL);

        int j = crypto_scalarmult_ristretto255(bin2, inv, bin);

        char hex[(crypto_core_ristretto255_BYTES * 2) + 1];
        sodium_bin2hex(hex, sizeof hex, bin2, crypto_core_ristretto255_BYTES);
        
        fprintf(file_out, "%s\n", hex);
    } 
    fclose(file_out);
    infile.close();
}
};
