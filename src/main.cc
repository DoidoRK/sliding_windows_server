#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include "config.h"
#include "types.h"
#include "utils/network_utils.h"
#include "utils/file_utils.h"
#include "sliding_windows.h"

using namespace std;

void uploadFile(const char *file_name){
    cout << "Fazendo Upload de: " << file_name << endl;
}


int main() {
    int server_socket, received_ftp_mode = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    operation_packet_t operation_packet;

    // Seed the random number generator with the current time to simulate time out occurring
    srand(static_cast<unsigned int>(time(nullptr)));

    check(
        (server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)),
        "Failed to create server's socket.\n"
    );

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    check(
        (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))),
        "Failed to bind server socket.\n"
    );

    while (1) {
        if(!received_ftp_mode){
            serverMessage(to_string(SERVER_PORT));
            cout << "Server is waiting for a client operation." << endl;
            check(
                (recvfrom(server_socket, &operation_packet, sizeof(operation_packet_t), 0, (struct sockaddr*)&client_addr, &client_addr_len)),
                "Failed to receive operation packet.\n"
            );
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, sizeof(clientIP));
            cout << "\n\nIncoming message from: " <<  clientIP << ":" << ntohs(client_addr.sin_port) << endl;
            cout << "FTP Mode: " << ((operation_packet.ftp_mode == 0) ? "Upload" : "Download") << " File: " << operation_packet.file_name << " Number of chunks: " << operation_packet.file_size_in_chunks << endl;

            switch (operation_packet.ftp_mode)
            {
                case DOWNLOAD:
                    received_ftp_mode = 1;
                    cout << "Cliente quer fazer download:" << operation_packet.file_name << endl;
                    // uploadFile(operation_packet.file_name);
                    break;

                case UPLOAD:
                    received_ftp_mode = 1;
                    cout << "Cliente quer fazer upload:" << operation_packet.file_name << endl;
                    downloadFile(operation_packet.file_name, server_socket, client_addr, operation_packet.file_size_in_chunks, &received_ftp_mode);
                    break;
                    
                default:
                    cout << "Invalid operation. Please use 'upload' or 'download'." << endl;
                    break;
            }
        }
    }

    close(server_socket);
    return 0;
}