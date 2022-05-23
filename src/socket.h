#include<iostream>
#include<fstream>
#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<chrono>
#include<cstring>
#include<ios>
#include<fstream>

using namespace std;

class logstreambuf: public streambuf{};
logstreambuf nostreambuf;
ostream nocout(&nostreambuf);

#define loglog(x) ((x > 0)? cout : nocout)

class Sender_socket{
    int PORT;
    
    int general_socket_descriptor;

    struct sockaddr_in address;
    int address_length;

    int bytes_sent;

    int verbose_level;

    public:
        Sender_socket(int port){
            #ifdef LOG
            verbose_level = 1;
            #else
            verbose_level = 0;
            #endif

            create_socket();
            PORT = port;

            address.sin_family = AF_INET;
            address.sin_port = htons( PORT );
            address_length = sizeof(address);
            if(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr)<=0) { 
                cout << "[ERROR] : Invalid address" << endl;
            }

            bytes_sent = 0;

            create_connection();
        }

        void create_socket(){
            if ((general_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
                perror("[ERROR] : Socket failed.\n");
                exit(EXIT_FAILURE);
            }
            loglog(verbose_level) << "[LOG] : Socket Created Successfully." << endl;
        }

        void create_connection(){
            if (connect(general_socket_descriptor, (struct sockaddr *)&address, sizeof(address)) < 0) {
                sleep(1);
                loglog(verbose_level) << "[LOG] : Could not connect yet." << endl;
                create_connection();
            }
            else {
                loglog(verbose_level) << "[LOG] : Connection Successfull." << endl;
            }
        }

        void transmit_file(string from){
            fstream file;

            file.open(from, ios::in | ios::binary);
            if(file.is_open()){
                loglog(verbose_level) << "[LOG] : File is ready to Transmit." << endl;
            }
            else{
                cout << "[ERROR] : File loading failed, Exititng." << endl;
                exit(EXIT_FAILURE);
            }

            std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            loglog(verbose_level) << "[LOG] : Transmission Data Size " << contents.length() << " Bytes." << endl;

            loglog(verbose_level) << "[LOG] : Sending..." << endl;


            const auto p1 = std::chrono::system_clock::now();
            long start = chrono::duration_cast<chrono::milliseconds>(p1.time_since_epoch()).count();

            string filename("./tmp/logfile.txt");
            ofstream file_out;

            file_out.open(filename, std::ios_base::app);
            file_out << "Sending: " << start << endl;

            bytes_sent = send(general_socket_descriptor , contents.c_str() , contents.length() , 0 );

            loglog(verbose_level) << "[LOG] : Transmitted Data Size " << bytes_sent << " Bytes." << endl;

            loglog(verbose_level) << "[LOG] : File Transfer Complete." << endl;

            if (bytes_sent == -1) {cout << "[ERROR] : " << errno << endl;}

            close(general_socket_descriptor);
        }

        int get_bytes_sent() {
            return bytes_sent;
        }
};

class Receiver_socket{
    int PORT;
    
    int general_socket_descriptor;
    int new_socket_descriptor;

    struct sockaddr_in address;
    int address_length;

    int total_received_bytes;

    int verbose_level;

    public:
        Receiver_socket(int port){
            #ifdef LOG
            verbose_level = 1;
            #else
            verbose_level = 0;
            #endif

            create_socket();
            PORT = port;

            address.sin_family = AF_INET; 
            address.sin_addr.s_addr = INADDR_ANY; 
            address.sin_port = htons( PORT );
            address_length = sizeof(address);

            total_received_bytes = 0;

            bind_socket();
            set_listen();
            accept_connection();
        }

        void create_socket(){
            if ((general_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
                perror("[ERROR] : Socket failed.\n");
                exit(EXIT_FAILURE);
            }
            loglog(verbose_level) << "[LOG] : Socket Created Successfully." << endl;
        }

        void bind_socket(){
            if (bind(general_socket_descriptor, (struct sockaddr *)&address, sizeof(address))<0) {
                perror("[ERROR] : Bind failed");
                exit(EXIT_FAILURE);
            }
            loglog(verbose_level) << "[LOG] : Bind Successful." << endl;
        }

        void set_listen(){
            if (listen(general_socket_descriptor, 1) < 0) {
                perror("[ERROR] : Listen");
                exit(EXIT_FAILURE);
            }
            loglog(verbose_level) << "[LOG] : Socket in Listen State" << endl;
        }

        void accept_connection(){
            if ((new_socket_descriptor = accept(general_socket_descriptor, (struct sockaddr *)&address, (socklen_t*)&address_length))<0) { 
                perror("[ERROR] : Accept");
                exit(EXIT_FAILURE);
            }
            loglog(verbose_level) << "[LOG] : Connected to Client." << endl;
        }

        void receive_file(string to){
            fstream file;

            file.open(to, ios::out | ios::trunc | ios::binary);
            if(file.is_open()){
                loglog(verbose_level) << "[LOG] : File Created." << endl;
            }
            else{
                cout << "[ERROR] : File creation failed, Exiting." << endl;
                exit(EXIT_FAILURE);
            }
            
            int size = 0;
            int received_bytes = 0;

            // #ifdef PRIME
            // size = 7500000;
            
            // char buffer[size];
            
            // while((received_bytes = read(new_socket_descriptor, buffer+total_received_bytes, size)) != - 1) {
            //     total_received_bytes += received_bytes;
            //     if(received_bytes == 0) break;
            //     loglog(verbose_level) << "[LOG] : Data received " << received_bytes << " bytes" << endl;
            // }
            // file<<buffer;
            // #else
            size = 75000;
            char buffer[size];
            
            while((received_bytes = read(new_socket_descriptor, buffer, size)) != - 1) {
                 total_received_bytes += received_bytes;
                 if(received_bytes == 0) break;
                 file<<buffer;
                 loglog(verbose_level) << "[LOG] : Data received " << received_bytes << " bytes" << endl;
                 memset(buffer, 0, sizeof(buffer));
            }
            // #endif

            loglog(verbose_level) << "[LOG] : Total data received " << total_received_bytes << " bytes" << endl;
            
            const auto p1 = std::chrono::system_clock::now();
            long stop = chrono::duration_cast<chrono::milliseconds>(p1.time_since_epoch()).count();

            string filename("./tmp/logfile.txt");
            ofstream file_out; 

            file_out.open(filename, std::ios_base::app);
            file_out << "Receiving: " << stop << endl;
            if (received_bytes == -1) { cout<<"[ERROR] : " << errno << endl; }
            else {
                loglog(verbose_level) << "[LOG] : File Saved." << endl;
            }
        }

        int get_received_bytes() {
            return total_received_bytes;
        }
};