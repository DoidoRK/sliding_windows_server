#ifndef _TYPES_H_
#define _TYPES_H_

#include <iostream>
#include "config.h"

using namespace std;

enum FTP_MODES {
    DOWNLOAD,
    UPLOAD,
};

enum FRAME_STATUS {
    ACKNOWLEDGED,
    NOT_ACKNOWLEDGED,
};
typedef struct frame_struct {
    int status;
    char data[CHUNK_SIZE];
} frame_t;

typedef struct operation_packet_struct {
    int ftp_mode;
    char file_path[FILE_PATH_SIZE];
    size_t number_of_chunks;
} operation_packet_t;

typedef struct data_packet_struct {
    int sequence_number;
    frame_t frame;
} data_packet_t;

#endif /* _TYPES_H_ */