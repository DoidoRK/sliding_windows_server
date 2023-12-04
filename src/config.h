#ifndef _CONFIG_H_
#define _CONFIG_H_

//Connection settings
#define SERVER_PORT 4000

//Socket timeout settings
#define SOCKET_TIMEOUT_IN_SECONDS 0
#define SOCKET_TIMEOUT_IN_MICROSSECONDS 10
//If time out time is lesser than
// WINDOW_SIZE * DELAY_BETWEEN_THREAD_CREATION
//Unexpected behaviour may appear

//Error simulation settings
#define CHANCE_FOR_ERROR_IN_SEND_PERCENT  20
#define CHANCE_FOR_ERROR_IN_RECV_PERCENT  20

//Sliding Windows settings
#define WINDOW_SIZE 10
#define FILE_NAME_SIZE 128
#define CHUNK_SIZE 128 //In Bytes
#define DELAY_BETWEEN_THREAD_CREATION 10

#define FILE_PATH "/home/doidobr/Projetos/Redes/sliding_windows_server/src/files/"

#endif /* _CONFIG_H_ */