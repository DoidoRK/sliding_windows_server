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
#include "utils/time_utils.h"
#include "utils/print_utils.h"
#include <chrono>

pthread_mutex_t current_chunk_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t window_end_chunk_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t frame_list_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t sliding_window[WINDOW_SIZE];

frame_t* frame_list = NULL;
int* is_running = NULL;
size_t number_of_chunks_in_file_in_file = 0;
size_t current_chunk = 0, window_end_chunk = 0;

void loadFramesFromFile(const char file_name[FILE_NAME_SIZE], size_t number_of_chunks_in_file) {
    FILE *file = fopen(file_name, "rb"); // Open the file in binary mode for reading
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Dynamically allocate memory for the frame list
    frame_list = (frame_t *)malloc(number_of_chunks_in_file * sizeof(frame_t));
    if (frame_list == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    // Read frames from the file into the dynamically allocated frame list
    for (size_t i = 0; i < number_of_chunks_in_file; ++i) {
        frame_list[i].status = NOT_ACKNOWLEDGED; // Start frames as not acknowledged

        // Read data from the file into the data property
        size_t bytes_read = fread(frame_list[i].data, sizeof(char), CHUNK_SIZE, file);

        if (bytes_read < CHUNK_SIZE) {
            // Fill the remaining bytes with empty spaces:
            for (size_t j = bytes_read; j < CHUNK_SIZE; ++j) {
                frame_list[i].data[j] = 32;
            }
        }
    }

    fclose(file); // Close the file after reading
}

void writeFileFromFrames(const char file_path[FILE_NAME_SIZE], size_t number_of_chunks_in_file) {
    FILE *file = fopen(file_path, "wb"); // Open the file in binary mode for writing
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Write frames from the frame list to the file
    for (size_t i = 0; i < number_of_chunks_in_file; ++i) {
        size_t bytesWritten = fwrite(frame_list[i].data, sizeof(char), CHUNK_SIZE, file);

        if (bytesWritten < CHUNK_SIZE) {
            perror("Error writing to file");
            fclose(file);
            return;
        }
    }

    fclose(file); // Close the file after writing
}

//Upload Proccess
void uploadFile(const char *file_path);



//Download Proccess
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
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(thread_port);

    check(
        (bind(thread_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))),
        "Failed to bind download thread socket.\n"
    );

    while (*is_running)
    {
        check(
            (recvfrom(thread_socket, &recv_packet, sizeof(data_packet_t), 0, (struct sockaddr*)&client_addr, &client_addr_len)),
            "Download thread failed to receive recv packet.\n"
        );
        printDataPacket(current_chunk,number_of_chunks_in_file_in_file,thread_port,recv_packet, RECV_DATA_PACKET);

        ack_packet.sequence_number = recv_packet.sequence_number;
        ack_packet.frame.status = ACKNOWLEDGED;

        frame_list[current_chunk] = recv_packet.frame;  //For window size equal to 1 to test

        int time_out_chance = generateRandomNumber();
        if(time_out_chance > TIMEOUT_CHANCE_PERCENT){
            printDataPacket(current_chunk,number_of_chunks_in_file_in_file,thread_port,ack_packet, SEND_DATA_PACKET);
            check(
                (sendto(thread_socket, &ack_packet, sizeof(data_packet_t), 0, (struct sockaddr*)&client_addr, sizeof(client_addr))),
                "Download thread failed to send ack packet.\n"
            );
            if(current_chunk == (number_of_chunks_in_file_in_file - 1)){
                *is_running = 0;
                cout << "killing download thread: " << thread_port << endl;
                cout << "File transfer finished." << endl;
                close(thread_socket);
            }
        } else {
            printTimeOutError(ack_packet.sequence_number);
        }
    }
    return 0;
}

void downloadFile(char file_name[FILE_NAME_SIZE], int server_socket, struct sockaddr_in client_addr, size_t number_of_chunks_in_file, int *received_ftp_mode){
    operation_packet_t operation_packet;

    //Gets file size.
    number_of_chunks_in_file_in_file = number_of_chunks_in_file;

    //Prepares file buffer to receive incoming data.
    frame_list = (frame_t *)malloc(number_of_chunks_in_file * sizeof(frame_t));
    is_running = received_ftp_mode;
    operation_packet.ftp_mode = UPLOAD;
    operation_packet.number_of_chunks_in_file = number_of_chunks_in_file;
    strcpy(operation_packet.file_name,file_name);

    //Creates server packets to handle data packets
    for (size_t i = 0; i < WINDOW_SIZE; i++)
    {
        uint16_t thread_port = SERVER_PORT + 1 + i;
        if (pthread_create(&sliding_window[i], NULL, downloadFileThread, (void*)&thread_port) != 0) {
            perror("Error creating download thread.\n");
            return;
        }
        usleep(DELAY_BETWEEN_THREAD_CREATION);
    }

    // Tells the client that it can start sending the data packets
    check(
        (sendto(server_socket, &operation_packet, sizeof(operation_packet_t), 0, (struct sockaddr*)&client_addr, sizeof(client_addr))),
        "Failed to send ack operation datagram.\n"
    );

    //When all the file data is received, write the file
    while (*is_running){}
    string file_name_string(file_name);
    string file_path = FILE_PATH + file_name_string;
    writeFileFromFrames(file_path.c_str(), number_of_chunks_in_file);
    free(frame_list);
}

#endif /* _SLIDING_WINDOWS_H_ */