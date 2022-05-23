#include"socket.h"

#include"DH_ecc.h"
#include"DH_prime.h"
#include"DH_unbalanced.h"
#include"DH_cuckoo.h"

#ifndef UTIL_H
#include"util.h"
#define UTIL_H
#endif

#include <chrono>
#include <math.h>

#ifdef PRIME
mpz_t key;
#endif


vector<string> available_sets_client = {"2", "10", "12", "14", "16", "18", "20"};
vector<string> available_sets_server = {"2", "12", "14", "16", "18", "20", "22"};

int main(int argc, char *argv[]){
    if (argc != 3) {
        cout << "Wrong number of arguments! \nMust input sample size of client and server" << endl;
        return 0;
    }

    bool found_client = (find(available_sets_client.begin(), available_sets_client.end(), argv[1]) != available_sets_client.end());
    bool found_server = (find(available_sets_server.begin(), available_sets_server.end(), argv[2]) != available_sets_server.end());


    if (!(found_client) ) {
        cout << "Client sample with size " << argv[1] << " not found" << endl;
        return 0;
    } else if ((!(found_server))) {
        cout << "Server sample with size " << argv[2] << " not found" << endl;
        return 0;   
    }

    int client_size = pow(2, atoi(argv[1]));
    int server_size = pow(2, atoi(argv[2]));

    ostringstream oss;
    oss << "tmp/samples/" << argv[1] << "_alice_samples.txt"; 
    string data_loc = oss.str();

    #ifdef PRIME
    prime::gen_key(key);
    #else
    unsigned char key[crypto_core_ristretto255_SCALARBYTES];
    crypto_core_ristretto255_scalar_random(key);
    #endif
    
    Sender_socket S(8050);
    auto start = chrono::high_resolution_clock::now();

    #ifdef ECC
    ecc::hash_and_op(data_loc, "./tmp/a.txt", key, true);
    #elif UB
    unbalanced::client_hash_and_op(data_loc, "./tmp/a.txt", key);
    #elif CUCKOO
    cuckoo::client_hash_and_op(data_loc, "./tmp/a.txt", key);
    #elif PRIME
    prime::hash_and_op(data_loc, "./tmp/a.txt", key, true);
    #endif

    S.transmit_file("./tmp/a.txt");

    Receiver_socket R1(8060);
    R1.receive_file("./tmp/network_b.txt");

    Receiver_socket R2(8070);
    R2.receive_file("./tmp/network_c.txt");

    #ifdef ECC
    ecc::hash_and_op("./tmp/network_b.txt", "./tmp/d.txt", key, false);
    #elif UB
    unbalanced::invert_and_hash("./tmp/network_c.txt", "./tmp/d.txt", key); 
    #elif CUCKOO
    cuckoo::invert("./tmp/network_c.txt", "./tmp/d.txt", key); 
    #elif PRIME
    prime::hash_and_op("./tmp/b.txt", "./tmp/d.txt", key, false);
    #endif
    

    #if defined(PRIME) || defined(ECC)
    util::intersection("./tmp/intersection.txt", "./tmp/network_c.txt", "./tmp/d.txt", data_loc, client_size, server_size);
    #elif defined(UB)
    util::intersection("./tmp/intersection.txt", "./tmp/d.txt", "./tmp/network_b.txt", data_loc, client_size, server_size);
    #elif defined(CUCKOO)
    cuckoo::read_and_intersect("./tmp/b.txt", "./tmp/d.txt", data_loc, client_size, server_size);
    #endif

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    int data_transfered = S.get_bytes_sent() + R1.get_received_bytes() + R2.get_received_bytes();

    cout << endl << "Protocol finished!" << endl;
    cout << "Total time elapsed: " << duration.count() << " milliseconds" << endl;
    
    cout << "Data transfered: " << data_transfered << " bytes" << endl;

    string filename("./tmp/logfile.txt");
    ofstream file_out; 
    file_out.open(filename, std::ios_base::app);
    
    file_out << "Time elapsed: " << duration.count() << endl;

    return 0;
}
