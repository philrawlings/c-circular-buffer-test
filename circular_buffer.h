
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
    int original_write_pos = write_pos;
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

int buffer_read(uint8_t* data, int max_length, uint32_t *start_pos) {
    int original_read_pos = read_pos;
    int bytes_available;
    
    if (write_pos >= read_pos) {
        bytes_available = write_pos - read_pos;
    }
    else {
        bytes_available = BUFFER_SIZE + write_pos - read_pos;
    }

    if (bytes_available == 0) {
        return 0;
    }

    int bytes_to_read = max_length < bytes_available ? max_length : bytes_available;
    if (bytes_to_read == 0) {
        return 0;
    }

    int bytes_to_end = BUFFER_SIZE - read_pos;
    if (bytes_to_read <= bytes_to_end) {
        memcpy(data, &buffer[read_pos], bytes_to_read);
    }
    else {
        int start_bytes_to_read = bytes_to_read - bytes_to_end;
        memcpy(data, &buffer[read_pos], bytes_to_end);
        memcpy(&data[bytes_to_end], buffer, start_bytes_to_read);
    }
    read_pos += bytes_to_read;
    read_pos %= BUFFER_SIZE;
    *start_pos = original_read_pos;
    return bytes_to_read;
}

#endif // APP_CIRCULAR_BUFFER_H_INCLUDED