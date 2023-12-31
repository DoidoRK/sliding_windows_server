#ifndef _PRINT_UTILS_H_
#define _PRINT_UTILS_H_
#include <pthread.h>
#include "../types.h"
#include "../libs/conio_linux.h"

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void printDataPacket(size_t window_start, size_t window_end, size_t total_size, uint16_t thread_port, data_packet_t data_packet, int data_packet_type){
    pthread_mutex_lock(&print_mutex);
    if(data_packet_type == RECV_DATA_PACKET){
        //Is a received packet
        setfontcolor(YELLOW);
        cout << "Thread Port: " << thread_port << " Receiving Data Packet" << endl;
    } else {
        //Is a packet to send
        setfontcolor(GREEN);
        cout << "Thread Port: " << thread_port << " Sending Data Packet" << endl;
    }
    cout << "Current Index: " << window_start << " Window End: " << window_end << endl;
    cout << " Packet info: ";
    cout << "Status: " << (data_packet.frame.status? "ACKNOWLEDGED" : "NOT_ACKNOWLEDGED");
    cout << " Sequence Number: " << data_packet.sequence_number << " / "  << total_size << endl << endl;
    setfontcolor(WHITE);
    pthread_mutex_unlock(&print_mutex);
}

void printOperationPacket(uint16_t thread_port, operation_packet_t operation_packet){
    pthread_mutex_lock(&print_mutex);
    cout << "Operation Packet:" << endl;
    cout << "FTP Mode: " << ((operation_packet.ftp_mode == 0) ? "Upload" : "Download");
    cout << " File: " << operation_packet.file_name;
    cout << " Number of chunks: " << operation_packet.file_size_in_chunks << endl;
    pthread_mutex_unlock(&print_mutex);
}

void printTimeOutError(uint16_t thread_port, size_t sequence_number){
    pthread_mutex_lock(&print_mutex);
    setfontcolor(CYAN);
    cout << "Thread in port: " << thread_port << endl;
    cout << "Waiting packet for sequence number: "<< sequence_number << " Timeout occurred!" << endl;
    setfontcolor(WHITE);
    pthread_mutex_unlock(&print_mutex);
}

void printAckTimeOutError(uint16_t thread_port, size_t sequence_number){
    pthread_mutex_lock(&print_mutex);
    setfontcolor(RED);
    cout << "Thread in port: " << thread_port << endl;
    cout << "Ack packet for sequence number: "<< sequence_number << " Timeout occurred!" << endl;
    setfontcolor(WHITE);
    pthread_mutex_unlock(&print_mutex);
}

void printSendError(uint16_t thread_port, size_t sequence_number){
    pthread_mutex_lock(&print_mutex);
    setfontcolor(MAGENTA);
    cout << "Thread in port: " << thread_port << endl;
    cout << "Failed to send packet for sequence number: "<< sequence_number << endl;
    setfontcolor(WHITE);
    pthread_mutex_unlock(&print_mutex);
}

#endif /* _PRINT_UTILS_H_ */