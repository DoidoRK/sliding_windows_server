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

pthread_mutex_t current_index_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t window_end_index_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t frame_list_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t sliding_window[WINDOW_SIZE];

frame_t* frame_list = NULL;
int* is_running = NULL;
size_t frame_list_last_index = 0;
size_t current_index_index = 0, window_end_index = 0;

void loadFramesFromFile(const char file_name[FILE_NAME_SIZE], size_t frame_list_last_index) {
    FILE *file = fopen(file_name, "rb"); // Open the file in binary mode for reading
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Dynamically allocate memory for the frame list
    frame_list = (frame_t *)malloc(frame_list_last_index * sizeof(frame_t));
    if (frame_list == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    // Read frames from the file into the dynamically allocated frame list
    for (size_t i = 0; i < frame_list_last_index; ++i) {
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

void writeFileFromFrames(const char file_path[FILE_NAME_SIZE], size_t frame_list_last_index) {
    FILE *file = fopen(file_path, "wb"); // Open the file in binary mode for writing
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Write frames from the frame list to the file
    for (size_t i = 0; i < frame_list_last_index; ++i) {
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
    int thread_socket, thread_status = WAITING_FOR_DATA;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    data_packet_t recv_packet, ack_packet;

    check(
        (thread_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)),
        "Failed to create server's socket.\n"
    );

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(thread_port);

    struct timeval timeout;
    timeout.tv_sec = SOCKET_TIMEOUT_IN_SECONDS;
    timeout.tv_usec = SOCKET_TIMEOUT_IN_MICROSSECONDS;
    check(
        (setsockopt (thread_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))),
        "Failed to set download thread socket receive timeout"
    );

    check(
        (bind(thread_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))),
        "Failed to bind download thread socket.\n"
    );

    int sequence_number = 0;
    int send_ack_success_chance;
    int recv_result;
    while (*is_running)
    {
        switch (thread_status)
        {
            case WAITING_FOR_DATA:
                recv_result = recvfrom(thread_socket, &recv_packet, sizeof(data_packet_t), 0, (struct sockaddr*)&client_addr, &client_addr_len);
                if (recv_result > 0) {
                    printDataPacket(frame_list_last_index, thread_port, recv_packet, RECV_DATA_PACKET);
                    sequence_number = recv_packet.sequence_number;
                    if(window_end_index - current_index_index < WINDOW_SIZE){ //Checks if sequence number is inside window
                        pthread_mutex_lock(&window_end_index_mutex);
                        window_end_index++;
                        pthread_mutex_unlock(&window_end_index_mutex);
                    }
                    thread_status = OPERATION_IN_BUFFER;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // Timeout occurred
                        printTimeOutError(thread_port, sequence_number);
                    } else {
                        perror("Error receiving data");
                    }
                    pthread_mutex_lock(&window_end_index_mutex);
                    pthread_mutex_lock(&current_index_mutex);
                    //If there is a error receiving package ack, go back N
                    window_end_index = current_index_index;
                    pthread_mutex_unlock(&current_index_mutex);
                    pthread_mutex_unlock(&window_end_index_mutex);
                    thread_status = WAITING_FOR_DATA;
                }
                break;

            case OPERATION_IN_BUFFER:
                    cout << "Entrou no OPERATION_IN_BUFFER" << endl;
                    pthread_mutex_lock(&frame_list_mutex);
                    //Current chunk receives incoming data and is set to acknowledged.
                    frame_list[ack_packet.sequence_number] = recv_packet.frame;  
                    frame_list[ack_packet.sequence_number].status = ACKNOWLEDGED;
                    pthread_mutex_unlock(&frame_list_mutex);
                    thread_status = SENDING_DATA;
                break;
            
            case SENDING_DATA:
                //Sends ACK
                cout << "Entrou no SENDING_DATA" << endl;
                ack_packet.sequence_number = sequence_number;
                ack_packet.frame.status = ACKNOWLEDGED;
                printDataPacket(frame_list_last_index, thread_port, ack_packet, SEND_DATA_PACKET);
                send_ack_success_chance = generateRandomNumber();
                if(send_ack_success_chance > ERROR_IN_COMM_CHANCE_PERCENT){
                    check(
                        (sendto(thread_socket, &ack_packet, sizeof(data_packet_t), 0, (struct sockaddr*)&client_addr, sizeof(client_addr))),
                        "Download thread failed to send ack packet.\n"
                    );
                    if(current_index_index == frame_list_last_index){
                        *is_running = 0;
                        cout << "killing download thread: " << thread_port << endl;
                        cout << "File transfer finished." << endl;
                        close(thread_socket);
                    }
                } else {
                    printSendError(thread_port,sequence_number);
                }
                thread_status = WAITING_FOR_DATA;
                break;

            default:
                cout << "Thread Status desconhecido";
                break;
        }
    }
    return 0;
}

void downloadFile(char file_name[FILE_NAME_SIZE], int server_socket, struct sockaddr_in client_addr, size_t file_size, int *received_ftp_mode){
    operation_packet_t operation_packet;

    //Gets file size.
    frame_list_last_index = file_size;

    //Prepares file buffer to receive incoming data.
    frame_list = (frame_t *)malloc(file_size * sizeof(frame_t));
    is_running = received_ftp_mode;
    operation_packet.ftp_mode = UPLOAD;
    operation_packet.file_size_in_chunks = file_size;
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
    writeFileFromFrames(file_path.c_str(), frame_list_last_index);
    free(frame_list);
}

#endif /* _SLIDING_WINDOWS_H_ */