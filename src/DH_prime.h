#include <sodium.h>
#include <gmp.h>
#include <string>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include<stdlib.h>
#include <time.h>
#include <random>
#include <limits>
#ifndef UTIL_H
#include"util.h"
#define UTIL_H
#endif
using namespace std;

namespace prime {

mpz_t primeq; // prime number of 2000 bits
mpz_t primep; // prime number equal to 2primeq + 1

mpz_t hash_value;
mpz_t op_value;

unsigned char hash_loc[10];

void get_primes(mpz_t loc1, mpz_t loc2) {
    ifstream file("./tmp/prime.txt");
    string str;

    getline(file, str);
    const char * c1 = str.c_str(); // reads number from file and sets it as const char *
    mpz_init_set_str(loc1, c1, 10);

    getline(file, str);
    const char * c2 = str.c_str(); // reads number from file and sets it as const char *
    mpz_init_set_str(loc2, c2, 10);
}

void bin2mpz(mpz_t loc, const unsigned char* hash) {
    // Puts binary output (hash-value) converted to hex at "loc"
    // Converted output of binary is 256 bits in size (64 (len) * 4 (size of hex char))
    char hex[(10 * 2) + 1];
    sodium_bin2hex(hex, sizeof hex, hash, 10);
    
    mpz_init_set_str(loc, hex, 16);
}

void gen_key(mpz_t loc) {
    get_primes(primeq, primep);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    
    std::random_device rd;
    std::mt19937_64 eng(rd());
    std::uniform_int_distribution<unsigned long long> distr;

    unsigned long seed = distr(eng);
    gmp_randseed_ui(state, seed);

    mpz_urandomm(loc, state, primeq);
}

void hash_and_op(string infilename, string outfilename, mpz_t key, bool do_hash) {
    get_primes(primeq, primep);
    
    ifstream infile(infilename);
    
    FILE *file_out;
    file_out = fopen(outfilename.c_str(), "wt");

    string line;
    
    if (do_hash) {
        while (getline(infile, line)) {
            const unsigned char* element = reinterpret_cast<const unsigned char*> (line.c_str());
            util::hash_element(hash_loc, element, strlen((const char*)element), 10);
            bin2mpz(hash_value, hash_loc);
            // Into order of q
            // mpz_mul(hash_value, hash_value, hash_value);
            mpz_powm(op_value, hash_value, key, primep);
            gmp_fprintf(file_out, "%Zx\n", op_value);
        }
    } else {
        while (getline(infile, line)) {
            mpz_t element_mpz;
            const char * c1 = line.c_str(); // reads number from file and sets it as const char *
            mpz_init_set_str(element_mpz, c1, 16);
            mpz_powm(op_value, element_mpz, key, primep);
            gmp_fprintf(file_out, "%Zx\n", op_value);
        }
    }
    fclose(file_out);
    infile.close();
}
};
