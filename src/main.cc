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

using namespace std;

void uploadFile(const char *file_name){
    cout << "Fazendo Upload de: " << file_name << endl;
}

void downloadFile(const char *file_name){
    cout << "Fazendo Download de: " << file_name << endl;
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addrLen = sizeof(client_addr);
    operation_packet_t operation_packet;

    check(
        (server_socket = socket(AF_INET, SOCK_DGRAM, 0)),
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
    serverMessage(to_string(SERVER_PORT));

    while (1) {
        check(
            (recvfrom(server_socket, &operation_packet, sizeof(operation_packet_t), 0, (struct sockaddr*)&client_addr, &client_addrLen)),
            "Failed to receive operation packet.\n"
        );
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, sizeof(clientIP));
        cout << "\n\nIncoming message from: " <<  clientIP << ":" << ntohs(client_addr.sin_port) << endl;
        cout << "FTP Mode: " << ((operation_packet.ftp_mode == 0) ? "Upload" : "Download") << " File: " << operation_packet.file_path << " Number of chunks: " << operation_packet.number_of_chunks << endl;

        switch (operation_packet.ftp_mode)
        {
            case DOWNLOAD:
                cout << "Cliente quer fazer download:" << operation_packet.file_path << endl;
                // uploadFile(operation_packet.file_path.c_str());
                break;

            case UPLOAD:
                cout << "Cliente quer fazer upload:" << operation_packet.file_path << endl;
                // downloadFile(server_file_path);
                break;
                
            default:
                cout << "Invalid operation. Please use 'upload' or 'download'." << endl;
                break;
        }

        check(
            (sendto(server_socket, &operation_packet, sizeof(operation_packet_t), 0, (struct sockaddr*)&client_addr, sizeof(client_addr))),
            "Failed to send ack operation datagram.\n"
        );
        cout << "Operation Ack sent." << endl;
    }

    close(server_socket);
    return 0;
}