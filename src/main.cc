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

using namespace std;

//Thread pool to reduce server memory usage.
pthread_t window_element_thread[WINDOW_SIZE];

//Creates sliding windows threads to deal with file transfer.
// for (uint8_t i = 0; i < WINDOW_SIZE; i++)
// {
//     pthread_create(&window_element_thread[i],NULL, connectionThread, NULL);
// }

void uploadFile(const char *file_name){
    cout << "Fazendo Upload de: " << file_name << endl;
}

void downloadFile(const char *file_name){
    cout << "Fazendo Download de: " << file_name << endl;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    operation_datagram_t operation_packet;

    check(
        (serverSocket = socket(AF_INET, SOCK_DGRAM, 0)),
        "Failed to create server's socket.\n"
    );

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    check(
        (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))),
        "Failed to bind server socket.\n"
    );
    serverMessage(to_string(SERVER_PORT));

    while (1) {
        check(
            (recvfrom(serverSocket, &operation_packet, sizeof(operation_datagram_t), 0, (struct sockaddr*)&clientAddr, &clientAddrLen)),
            "Failed to receive operation packet.\n"
        );
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
        printf("Mensagem recebida de: %s:%d\n\n", clientIP, ntohs(clientAddr.sin_port));
        printf("Operação: %s\nArquivo: %s\n", operation_packet.operation, operation_packet.file_path);
    }

    close(serverSocket);
    return 0;
}