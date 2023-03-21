
// Header only library
#include <stdio.h>
#include <string.h>

#ifndef APP_CIRCULAR_BUFFER_H_INCLUDED
#define APP_CIRCULAR_BUFFER_H_INCLUDED

#define BUFFER_SIZE 80

uint8_t buffer[BUFFER_SIZE];

int read_pos = 0;
int write_pos = 0;

void buffer_init() {
    read_pos = 0;
    write_pos = 0;
    memset(buffer, 0, sizeof(buffer));
}

int buffer_write(uint8_t* data, int length) {
    int end_pos = write_pos + length;
    if (end_pos < BUFFER_SIZE) {
        memcpy(&buffer[write_pos], data, length);
    }
    else {
        int start_bytes_to_write = end_pos - BUFFER_SIZE;
        int end_bytes_to_write = length - start_bytes_to_write;
        memcpy(&buffer[write_pos], &data[0], end_bytes_to_write); // Write to end of buffer
        memcpy(buffer, &data[end_bytes_to_write], start_bytes_to_write); // Write to start of buffer
    }
    write_pos = end_pos % BUFFER_SIZE;
    // Check if read_pos has been lapped
    if (write_pos > read_pos && write_pos - read_pos < length) {
        return -1;
    }
    else {
        return length;
    }
}

int buffer_read_from_pos(uint8_t* data, int max_length, uint32_t start_pos, uint32_t *next_start_pos) {
    int bytes_available;
    
    if (write_pos >= start_pos) {
        bytes_available = write_pos - start_pos;
    }
    else {
        bytes_available = BUFFER_SIZE + write_pos - start_pos;
    }

    if (bytes_available == 0) {
        return 0;
    }

    int bytes_to_read = max_length < bytes_available ? max_length : bytes_available;
    if (bytes_to_read == 0) {
        return 0;
    }

    int bytes_to_end = BUFFER_SIZE - start_pos;
    if (bytes_to_read <= bytes_to_end) {
        memcpy(data, &buffer[start_pos], bytes_to_read);
    }
    else {
        int start_bytes_to_read = bytes_to_read - bytes_to_end;
        memcpy(data, &buffer[start_pos], bytes_to_end);
        memcpy(&data[bytes_to_end], buffer, start_bytes_to_read);
    }

    *next_start_pos = (start_pos + bytes_to_read) % BUFFER_SIZE;
    read_pos = *next_start_pos;
    return bytes_to_read;
}

int buffer_read_next(uint8_t* data, int max_length) {
    uint32_t next_start_pos;
    return buffer_read_from_pos(data, max_length, read_pos, &next_start_pos);
}

#endif // APP_CIRCULAR_BUFFER_H_INCLUDED