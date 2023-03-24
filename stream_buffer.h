
// Header only library
#include <stdio.h>
#include <string.h>

#ifndef APP_STREAM_BUFFER_H_INCLUDED
#define APP_STREAM_BUFFER_H_INCLUDED

#define BUFFER_SIZE 80

uint8_t buffer[BUFFER_SIZE];
uint32_t byte_count = 0;
uint32_t read_pos = 0;
uint32_t write_pos = 0;

void stream_buffer_init() {
    byte_count = 0;
    read_pos = 0;
    write_pos = 0;
    memset(buffer, 0, sizeof(buffer));
}

// Returns bytes written
uint32_t stream_buffer_write(uint8_t* data, uint32_t length) {
    uint32_t next_write_pos = (write_pos + length) % BUFFER_SIZE;
    if (next_write_pos >= read_pos && next_write_pos - read_pos < length) {
        return 0; // read_pos has been lapped: buffer overflow
    }

    if (next_write_pos > write_pos) {
        memcpy(&buffer[write_pos], data, length);
    }
    else {
        uint32_t start_bytes_to_write = write_pos + length - BUFFER_SIZE;
        uint32_t end_bytes_to_write = length - start_bytes_to_write;
        memcpy(&buffer[write_pos], &data[0], end_bytes_to_write); // Write to end of buffer
        memcpy(buffer, &data[end_bytes_to_write], start_bytes_to_write); // Write to start of buffer
    }

    write_pos = next_write_pos;
    if (byte_count < BUFFER_SIZE) {
        uint32_t available = (byte_count + length);
        byte_count = available < BUFFER_SIZE ? available : BUFFER_SIZE;
    }
    
    return length;
}

// Returns bytes read
uint32_t stream_buffer_read_from_pos(uint8_t* data, uint32_t max_length, uint32_t start_pos, uint32_t *next_start_pos) {
    uint32_t bytes_available;
    
    if (write_pos >= start_pos) {
        bytes_available = write_pos - start_pos;
    }
    else {
        bytes_available = BUFFER_SIZE + write_pos - start_pos;
    }

    if (bytes_available == 0) {
        *next_start_pos = read_pos;
        return 0;
    }

    uint32_t bytes_to_read = max_length < bytes_available ? max_length : bytes_available;
    if (bytes_to_read == 0) {
        *next_start_pos = read_pos;
        return 0;
    }

    uint32_t bytes_to_end = BUFFER_SIZE - start_pos;
    if (bytes_to_read <= bytes_to_end) {
        memcpy(data, &buffer[start_pos], bytes_to_read);
    }
    else {
        uint32_t start_bytes_to_read = bytes_to_read - bytes_to_end;
        memcpy(data, &buffer[start_pos], bytes_to_end);
        memcpy(&data[bytes_to_end], buffer, start_bytes_to_read);
    }

    *next_start_pos = (start_pos + bytes_to_read) % BUFFER_SIZE;
    read_pos = *next_start_pos;
    return bytes_to_read;
}

// Returns bytes read
uint32_t stream_buffer_read_next(uint8_t* data, uint32_t max_length) {
    uint32_t next_start_pos;
    return stream_buffer_read_from_pos(data, max_length, read_pos, &next_start_pos);
}

#endif // APP_STREAM_BUFFER_H_INCLUDED