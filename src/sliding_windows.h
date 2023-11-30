#ifndef _SLIDING_WINDOWS_H_
#define _SLIDING_WINDOWS_H_

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

pthread_mutex_t frame_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t current_chunk_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t sliding_window[WINDOW_SIZE];

frame_t* frame_list = NULL;
int* is_running = NULL;
size_t number_of_chunks_in_file = 0;
size_t current_chunk = 0;

void loadFramesFromFile(const char file_name[FILE_NAME_SIZE], size_t number_of_chunks) {
    FILE *file = fopen(file_name, "rb"); // Open the file in binary mode for reading
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Dynamically allocate memory for the frame list
    frame_list = (frame_t *)malloc(number_of_chunks * sizeof(frame_t));
    if (frame_list == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    // Read frames from the file into the dynamically allocated frame list
    for (size_t i = 0; i < number_of_chunks; ++i) {
        frame_list[i].status = NOT_ACKNOWLEDGED; // Start frames as not acknowledged

        // Read data from the file into the data property
        size_t bytesRead = fread(frame_list[i].data, sizeof(char), CHUNK_SIZE, file);

        if (bytesRead < CHUNK_SIZE) {
            // Fill the remaining bytes with zeros:
            for (size_t j = bytesRead; j < CHUNK_SIZE; ++j) {
                frame_list[i].data[j] = 0;
            }
        }
    }

    fclose(file); // Close the file after reading
}

void uploadFile(const char *file_path);



void* downloadFileThread(void* arg){
    uint16_t thread_port = *((uint16_t*)arg);
    int thread_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    data_packet_t recv_packet, ack_packet;

    check(
        (thread_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)),
        "Failed to create server's socket.\n"
    );

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(thread_port);
    
    // struct timeval timeout;
    // timeout.tv_sec = SOCKET_TIMEOUT_IN_SECONDS;
    // timeout.tv_usec = SOCKET_TIMEOUT_IN_MICROSSECONDS;
    // check(
    //     (setsockopt (thread_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof(timeout))),
    //     "Failed to set thread socket receive timeout"
    // );

    // int reuse = 1;
    // if (setsockopt(thread_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
    //     perror("setsockopt(SO_REUSEADDR) failed");
    //     return 0;
    // }

    check(
        (bind(thread_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))),
        "Failed to bind thread socket.\n"
    );

    while (*is_running)
    {
        check(
            (recvfrom(thread_socket, &recv_packet, sizeof(data_packet_t), 0, (struct sockaddr*)&client_addr, &client_addr_len)),
            "Download thread failed to receive recv packet.\n"
        );
        cout << "thread port: " << thread_port << " received data packet!" << endl;
        cout << "Packet contents:" << endl;
        cout << (recv_packet.frame.status? "ACKNOWLEDGED" : "NOT_ACKNOWLEDGED") << endl;
        cout << recv_packet.sequence_number << endl;

        ack_packet.sequence_number = recv_packet.sequence_number;
        ack_packet.frame.status = ACKNOWLEDGED;
        current_chunk = ack_packet.sequence_number; //For window size equal to 1 to test

        frame_list[current_chunk] = recv_packet.frame;  //For window size equal to 1 to test
        // pthread_mutex_lock(&current_chunk_mutex);
        // pthread_mutex_unlock(&current_chunk_mutex);
        check(
            (sendto(thread_socket, &ack_packet, sizeof(data_packet_t), 0, (struct sockaddr*)&client_addr, sizeof(client_addr))),
            "Download thread failed to send ack packet.\n"
        );
        if(current_chunk > number_of_chunks_in_file){
            *is_running = 0;
        }
    }
    return 0;
}

void downloadFile(char file_name[FILE_NAME_SIZE], int server_socket, struct sockaddr_in client_addr, size_t number_of_chunks, int *received_ftp_mode){
    operation_packet_t operation_packet;

    //Gets file size.
    number_of_chunks_in_file = number_of_chunks;

    //Prepares file buffer to receive incoming data.
    frame_list = (frame_t *)malloc(number_of_chunks * sizeof(frame_t));
    is_running = received_ftp_mode;
    operation_packet.ftp_mode = UPLOAD;
    operation_packet.number_of_chunks = number_of_chunks;
    strcpy(operation_packet.file_name,file_name);

    //Creates server threads to handle data packets.
    for (uint8_t i = 0; i < WINDOW_SIZE; i++)
    {
        uint16_t thread_port = SERVER_PORT+(i+1);
        cout << "Assigned thread port: " << thread_port << endl;
         if (pthread_create(&sliding_window[i], NULL, downloadFileThread, (void*)&thread_port) != 0) {
            perror("Error creating download thread.\n");
            return;
        }
    }

    //Tells the client that it can start sending the data packets
    check(
        (sendto(server_socket, &operation_packet, sizeof(operation_packet_t), 0, (struct sockaddr*)&client_addr, sizeof(client_addr))),
        "Failed to send ack operation datagram.\n"
    );
}

#endif /* _SLIDING_WINDOWS_H_ */