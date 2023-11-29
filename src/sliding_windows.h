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

pthread_mutex_t sliding_window_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t current_chunk_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t sliding_window[WINDOW_SIZE];

frame_t** frame_list = NULL;
size_t number_of_chunks = 0;
size_t current_chunk = 0;

void uploadFile(const char *file_path);

void downloadFile(const char *file_name) {}

#endif /* _SLIDING_WINDOWS_H_ */