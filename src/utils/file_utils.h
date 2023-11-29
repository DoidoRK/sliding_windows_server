#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#include <stdio.h>
#include <iostream>
#include "../types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t calculateNumChunks(const char* file_path, size_t chunk_size) {
    FILE* file = fopen(file_path, "rb");

    if (!file) {
        perror("Error opening file");
        return 0; // Return 0 indicating an error
    }

    // Get the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    fclose(file);

    // Calculate the number of chunks
    size_t num_chunks = (file_size / chunk_size) + ((file_size % chunk_size != 0) ? 1 : 0);

    return num_chunks;
}

void initializeFrameList(frame_t** frame_list, size_t num_chunks) {
    frame_list = (frame_t**)malloc(num_chunks * sizeof(frame_t*));
    if (!frame_list) {
        perror("Error allocating memory for frame list");
        exit(EXIT_FAILURE);
    }

    // Initialize each element to NULL
    for (size_t i = 0; i < num_chunks; ++i) {
        frame_list[i] = NULL;
    }
}

void freeFrameList(frame_t** frame_list, size_t list_size) {
    for (size_t i = 0; i < list_size; ++i) {
        free(frame_list[i]);
    }
    free(frame_list);
}

void readFileToFrameList(const char* file_path, frame_t** frame_list, size_t list_size) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        perror("Error opening file");
        freeFrameList(frame_list, list_size);  // Free allocated memory before exiting
        exit(EXIT_FAILURE);
    }

    // Read the file into the frame_list
    for (size_t i = 0; i < list_size; ++i) {
        frame_list[i] = (frame_t*)malloc(sizeof(frame_t));
        if (!frame_list[i]) {
            perror("Error allocating memory for frame");
            fclose(file);
            freeFrameList(frame_list, list_size);  // Free allocated memory before exiting
            exit(EXIT_FAILURE);
        }

        frame_list[i]->status = NOT_ACKNOWLEDGED;

        // Read a chunk of data from the file into the frame
        size_t bytes_read = fread(frame_list[i]->data, 1, CHUNK_SIZE, file);
        if (bytes_read == 0) {
            // Handle the end of the file or an error
            break;
        }
    }

    fclose(file);
}

#endif /* _FILE_UTILS_H_ */