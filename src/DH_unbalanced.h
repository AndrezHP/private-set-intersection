#include<sodium.h>
#include<cstring> 
#include<cstdlib>
#include<sstream>
#ifndef UTIL_H
#include"util.h"
#define UTIL_H
#endif
using namespace std;

namespace unbalanced {

const int hash_size = 64;
const int hash_size2 = 10;

unsigned char hash_loc[hash_size];
unsigned char hash_loc2[hash_size2];

void server_hash_and_op(string infilename, string outfilename, unsigned char * key, bool do_hash) {
    unsigned char op_value[crypto_core_ristretto255_BYTES];
    unsigned char px[crypto_core_ristretto255_BYTES];
    unsigned char bin[crypto_core_ristretto255_BYTES];

    ifstream infile(infilename);

    FILE *file_out;
    file_out = fopen(outfilename.c_str(), "wt");

    string line;
    
    if (do_hash) {
        while (getline(infile, line)) {
            const unsigned char *element = reinterpret_cast<const unsigned char*> (line.c_str());
            util::hash_element(hash_loc, element, strlen((const char*)element), hash_size);
            crypto_core_ristretto255_from_hash(px, hash_loc);

            int i = crypto_scalarmult_ristretto255(op_value, key, px);

            util::hash_element(hash_loc2, op_value, sizeof(op_value), hash_size2);

            char hex[(hash_size2 * 2) + 1];
            sodium_bin2hex(hex, sizeof hex, hash_loc2, hash_size2);

            fprintf(file_out, "%s\n", hex);
        }
    } else {
        while (getline(infile, line)) {
            const unsigned char *element = reinterpret_cast<const unsigned char*> (line.c_str());
            int succ_h2b = sodium_hex2bin(bin, 32, (const char*)element, 64, NULL, NULL, NULL);
            int i = crypto_scalarmult_ristretto255(op_value, key, bin);
            char hex[(32 * 2) + 1];
            sodium_bin2hex(hex, sizeof hex, op_value, 32);
            fprintf(file_out, "%s\n", hex);
        }
    }
    
    fclose(file_out);
    infile.close();
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
        util::hash_element(hash_loc, element, strlen((const char*)element), hash_size);
        crypto_core_ristretto255_from_hash(px, hash_loc);

        char hex_temp[(32 * 2) + 1];
        sodium_bin2hex(hex_temp, sizeof hex_temp, hash_loc, 32);

        int i = crypto_scalarmult_ristretto255(op_value, key, px);
        
        char hex[(crypto_core_ristretto255_BYTES * 2) + 1];
        sodium_bin2hex(hex, sizeof hex, op_value, crypto_core_ristretto255_BYTES);

        fprintf(file_out, "%s\n", hex);
    } 
    fclose(file_out);
    infile.close();
}

void invert_and_hash(string infilename, string outfilename, unsigned char * key) {
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

        util::hash_element(hash_loc2, bin2, sizeof(bin2), hash_size2);
        char hex[(hash_size2 * 2) + 1];
        sodium_bin2hex(hex, sizeof hex, hash_loc2, hash_size2);

        fprintf(file_out, "%s\n", hex);
    } 
    fclose(file_out);
    infile.close();
}
};
