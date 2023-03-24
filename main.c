#include <stdio.h>
#include <stdint.h>
#include "stream_buffer.h"
#include <windows.h> // for sleep and threading functions

uint8_t val = 0;

void set_data(uint8_t* buffer, int length) {
	for (int i = 0; i < length; i++) {
		buffer[i] = val++;
	}
}

void verify_data(uint8_t* buffer, int length, int start_value) {
	for (int i = 0; i < length; i++) {
		if (buffer[i] != (start_value + i) % 256) {
			printf("Verify data failed.\n");
			exit(-1);
		}
	}
}

void assert_int(int expected, int actual) {
	if (expected != actual) {
		printf("Integer mismatch. Expected %d, found %d.\n", expected, actual);
		exit(-1);
	}
}

void simple_test() { 
	uint8_t buffer[100];
	int byte_count; 

	stream_buffer_init();

	// Write 64 bytes in 2 chunks
	set_data(buffer, 32);
	byte_count = stream_buffer_write(buffer, 32);
	assert_int(32, byte_count);
	set_data(buffer, 32);
	byte_count = stream_buffer_write(buffer, 32);
	assert_int(32, byte_count);

	// Read 64 bytes in 2 chunks
	byte_count = stream_buffer_read_next(buffer, 32);
	assert_int(32, byte_count);
	verify_data(buffer, 32, 0);
	byte_count = stream_buffer_read_next(buffer, 32);
	assert_int(32, byte_count);
	verify_data(buffer, 32, 32);

	// Write 64 bytes in 2 chunks
	set_data(buffer, 32);
	byte_count = stream_buffer_write(buffer, 32);
	assert_int(32, byte_count);
	set_data(buffer, 32);
	byte_count = stream_buffer_write(buffer, 32);
	assert_int(32, byte_count);

	// Read 64 bytes in chunks of 50 and 14
	byte_count = stream_buffer_read_next(buffer, 50);
	verify_data(buffer, 50, 64);
	assert_int(50, byte_count);
	byte_count = stream_buffer_read_next(buffer, 14);
	assert_int(14, byte_count);
	verify_data(buffer, 14, 114);

	// Write/read 60 bytes
	set_data(buffer, 60);
	byte_count = stream_buffer_write(buffer, 60);
	assert_int(60, byte_count);
	byte_count = stream_buffer_read_next(buffer, 60);
	assert_int(60, byte_count);
	verify_data(buffer, 60, 128);

	// Write/read 79 bytes
	set_data(buffer, 79);
	byte_count = stream_buffer_write(buffer, 79);
	assert_int(79, byte_count);
	byte_count = stream_buffer_read_next(buffer, 79);
	assert_int(79, byte_count);
	verify_data(buffer, 79, 188);

	// Write/read 79 bytes
	set_data(buffer, 79);
	byte_count = stream_buffer_write(buffer, 79);
	assert_int(79, byte_count);
	byte_count = stream_buffer_read_next(buffer, 79);
	assert_int(79, byte_count);
	verify_data(buffer, 79, 11); // Value rollover

	// Random access
	val = 0;
	stream_buffer_init();
	set_data(buffer, 10);
	byte_count = stream_buffer_write(buffer, 10);
	assert_int(10, byte_count);
	int next_start_pos;
	byte_count = stream_buffer_read_from_pos(buffer, 5, 5, &next_start_pos);
	assert_int(5, byte_count);
	assert_int(10, next_start_pos);
	verify_data(buffer, 5, 5);

	byte_count = stream_buffer_read_from_pos(buffer, 7, 2, &next_start_pos);
	assert_int(7, byte_count);
	assert_int(9, next_start_pos);
	verify_data(buffer, 7, 2);
	byte_count = stream_buffer_read_from_pos(buffer, 1, 9, &next_start_pos);
	assert_int(1, byte_count);
	assert_int(10, next_start_pos);
	verify_data(buffer, 1, 9);
	byte_count = stream_buffer_read_from_pos(buffer, 1, 10, &next_start_pos); // No new data to read
	assert_int(0, byte_count);
	assert_int(10, next_start_pos);

	// Buffer overflow
	val = 0;
	stream_buffer_init();
	set_data(buffer, 70);
	byte_count = stream_buffer_write(buffer, 70);
	assert_int(70, byte_count);
	set_data(buffer, 9);
	byte_count = stream_buffer_write(buffer, 9);
	assert_int(9, byte_count);
	set_data(buffer, 1);
	byte_count = stream_buffer_write(buffer, 1); // overflow
	assert_int(0, byte_count);

	// No data to read
	val = 0;
	stream_buffer_init();
	byte_count = stream_buffer_read_next(buffer, 50);
	assert_int(0, byte_count);
	set_data(buffer, 32);
	byte_count = stream_buffer_write(buffer, 32);
	assert_int(32, byte_count);
	byte_count = stream_buffer_read_next(buffer, 32);
	assert_int(32, byte_count);
	verify_data(buffer, 32, 0);
	byte_count = stream_buffer_read_next(buffer, 50);
	assert_int(0, byte_count);
	
	// No data to read (random access)
	val = 0;
	stream_buffer_init();
	byte_count = stream_buffer_read_from_pos(buffer, 50, 0, &next_start_pos);
	assert_int(0, byte_count);
	assert_int(0, next_start_pos);

	printf("Simple test complete!\n");
}

void loop_test() {
	val = 0;
	stream_buffer_init();
	uint8_t buffer[15];
	uint8_t start_value = 0;
	for (int i = 0; i < 1000000; i++) {
		set_data(buffer, sizeof(buffer));
		int byte_count = stream_buffer_write(buffer, sizeof(buffer));
		assert_int(sizeof(buffer), byte_count);
		byte_count = stream_buffer_read_next(buffer, sizeof(buffer));
		assert_int(sizeof(buffer), byte_count);
		verify_data(buffer, sizeof(buffer), start_value);
		start_value += sizeof(buffer);
	}
	printf("Loop test complete!\n");
}

DWORD WINAPI writer_thread(void *data) {
	uint8_t buffer[19];
	val = 0;
	while(1) {
		set_data(buffer, sizeof(buffer));
		if (stream_buffer_write(buffer, sizeof(buffer)) != sizeof(buffer)) {
			printf("Failed to write!\n");
			break;
		}
		Sleep(1);
	}
}

DWORD WINAPI reader_thread(void *data) {
	uint8_t buffer[50];
	uint8_t start_value = 0;
	while(1) {
		int byte_count = stream_buffer_read_next(buffer, sizeof(buffer));
		if (byte_count == 0) {
			Sleep(2);
		}
		else {
			verify_data(buffer, byte_count, start_value);
			start_value = start_value + byte_count;
		}
	}
}

void thread_test() {
	stream_buffer_init();
	HANDLE t_writer = CreateThread(NULL, 0, writer_thread, NULL, 0, NULL);
	HANDLE t_reader = CreateThread(NULL, 0, reader_thread, NULL, 0, NULL);
}

int main() {
	simple_test();
	loop_test();
	thread_test();
	Sleep(5000);
	printf("All tests complete!\n");
	return 0;
}