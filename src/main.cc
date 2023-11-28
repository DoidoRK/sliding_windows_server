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
#include "utils/network_utils.h"
#include "libs/conio_linux.h"

using namespace std;

//Thread pool to reduce server memory usage.
pthread_t thread_pool[WINDOW_SIZE];

int main() {

    int server_socket, client_socket;
    struct sockaddr_in server_addr;

    clrscr();

    // //Creates sliding windows threads to deal with file transfer.
    // for (uint8_t i = 0; i < WINDOW_SIZE; i++)
    // {
    //     pthread_create(&thread_pool[i],NULL, connectionThread, NULL);
    // }
    
    // check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "Failed to open stream socket");

    // /* Adresses */
    // bzero(&server_addr, sizeof(server_addr));
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    // server_addr.sin_port = htons(SERVER_PORT);
    // check((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0),"Failed to bind stream socket");
    
    // /* Waits for connections */
    // check((listen(server_socket,MAX_CONNECTIONS)), "Failed to listen");
    // if(-1 == serverMessage(to_string(SERVER_PORT))){
    //     cout << "Failed to get server IP address!" << endl;
    // };

    // while (1) {
    //     check((client_socket = accept(server_socket,(struct sockaddr *)0,0)),"Accept Failed");
    //     int *pclient = (int*)malloc(sizeof(int));
    //     *pclient = client_socket;
    // }
    return 0;
}