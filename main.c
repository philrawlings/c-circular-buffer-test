#include <stdio.h>
#include <stdint.h>
#include "circular_buffer.h"
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

	buffer_init();

	set_data(buffer, 32);
	byte_count = buffer_write(buffer, 32);
	assert_int(32, byte_count);
	set_data(buffer, 32);
	byte_count = buffer_write(buffer, 32);
	assert_int(32, byte_count);

	byte_count = buffer_read_next(buffer, 32);
	assert_int(32, byte_count);
	verify_data(buffer, 32, 0);
	byte_count = buffer_read_next(buffer, 32);
	assert_int(32, byte_count);
	verify_data(buffer, 32, 32);

	set_data(buffer, 32);
	byte_count = buffer_write(buffer, 32);
	assert_int(32, byte_count);
	set_data(buffer, 32);
	byte_count = buffer_write(buffer, 32);
	assert_int(32, byte_count);

	byte_count = buffer_read_next(buffer, 50);
	verify_data(buffer, 50, 64);
	assert_int(50, byte_count);
	byte_count = buffer_read_next(buffer, 14);
	assert_int(14, byte_count);
	verify_data(buffer, 14, 114);

	set_data(buffer, 60);
	byte_count = buffer_write(buffer, 60);
	assert_int(60, byte_count);
	byte_count = buffer_read_next(buffer, 60);
	assert_int(60, byte_count);
	verify_data(buffer, 60, 128);

	set_data(buffer, 79);
	byte_count = buffer_write(buffer, 79);
	assert_int(79, byte_count);
	byte_count = buffer_read_next(buffer, 79);
	assert_int(79, byte_count);
	verify_data(buffer, 79, 188);

	set_data(buffer, 79);
	byte_count = buffer_write(buffer, 79);
	assert_int(79, byte_count);
	byte_count = buffer_read_next(buffer, 79);
	assert_int(79, byte_count);
	verify_data(buffer, 79, 11); // Wrap

	val = 0;
	buffer_init();
	set_data(buffer, 10);
	byte_count = buffer_write(buffer, 10);
	assert_int(10, byte_count);
	int next_start_pos;
	byte_count = buffer_read_from_pos(buffer, 5, 5, &next_start_pos);
	assert_int(5, byte_count);
	assert_int(10, next_start_pos);
	verify_data(buffer, 5, 5);


	printf("Simple test complete!\n");
}

void loop_test() {
	val = 0;
	buffer_init();
	uint8_t buffer[15];
	uint8_t start_value = 0;
	for (int i = 0; i < 1000000; i++) {
		set_data(buffer, sizeof(buffer));
		int byte_count = buffer_write(buffer, sizeof(buffer));
		assert_int(sizeof(buffer), byte_count);
		byte_count = buffer_read_next(buffer, sizeof(buffer));
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
		if (buffer_write(buffer, sizeof(buffer)) != sizeof(buffer)) {
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
		int byte_count = buffer_read_next(buffer, sizeof(buffer));
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
	buffer_init();
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