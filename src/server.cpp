#include"socket.h"
#include<sodium.h>
#include<string.h>
#include<fstream>
#include"DH_ecc.h"
#include"DH_unbalanced.h"
#include"DH_cuckoo.h"
#ifdef PRIME
#include"DH_prime.h"
mpz_t key;
#endif
#include <chrono>
#include<algorithm>
#include <math.h>

#ifndef UTIL_H
#include"util.h"
#define UTIL_H
#endif

vector<string> available_sets = {"2", "12", "14", "16", "18", "20", "22"};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Wrong number of arguments! \nMust input sample size" << endl;
        return 0;
    }

    bool found = (find(available_sets.begin(), available_sets.end(), argv[1]) != available_sets.end());

    if (!found) {
        cout << "Sample of size " << argv[1] << " not found" << endl;
        return 0;
    }

    ostringstream oss;
    oss << "tmp/samples/" << argv[1] << "_bob_samples.txt"; 
    string data_loc = oss.str();
    int server_size = pow(2, atoi(argv[1]));

    #ifdef PRIME
    prime::gen_key(key);
    #else
    unsigned char key[crypto_core_ristretto255_SCALARBYTES];
    crypto_core_ristretto255_scalar_random(key);
    #endif
    
    cout << "[LOG] : Preprocessing..." << endl;
    auto start = chrono::high_resolution_clock::now();
    #ifdef ECC
    ecc::hash_and_op(data_loc, "./tmp/b.txt", key, true);
    #elif UB
    unbalanced::server_hash_and_op(data_loc, "./tmp/b.txt", key, true);
    #elif PRIME
    prime::hash_and_op(data_loc, "./tmp/b.txt", key, true);
    #elif CUCKOO
    cuckoo::server_hash_and_op(data_loc, "./tmp/b.txt", key, true, server_size);
    #endif

    auto stop = chrono::high_resolution_clock::now();
    auto preprocessing_duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    string filename("./tmp/logfile.txt");
    ofstream file_out; 

    file_out.open(filename, std::ios_base::app);
    file_out << "Preprocessing: " << preprocessing_duration.count() << endl;

    Receiver_socket R(8050);
    R.receive_file("./tmp/network_a.txt");

    Sender_socket S1(8060);
    S1.transmit_file("./tmp/b.txt");

    #ifdef ECC
    ecc::hash_and_op("./tmp/network_a.txt", "./tmp/c.txt", key, false);
    #elif UB
    unbalanced::server_hash_and_op("./tmp/network_a.txt", "./tmp/c.txt", key, false); 
    #elif PRIME
    prime::hash_and_op("./tmp/a.txt", "./tmp/c.txt", key, false);
    #elif CUCKOO
    cuckoo::server_hash_and_op("./tmp/network_a.txt", "./tmp/c.txt", key, false, 0); 
    #endif

    Sender_socket S2(8070);
    S2.transmit_file("./tmp/c.txt");

    return 0;
}

