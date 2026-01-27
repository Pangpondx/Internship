#pragma once
#include <stdint.h>

#define SYNC_BYTE_1 0x5A;
#define SYNC_BYTE_2 0xA5;
#define UART_DEVICE "/dev/ttyS1"
#define UART_BAUDRATE 115200
//#define LE16(p) ((uint16_t)((p)[0] | ((p)[1] << 8)))

typedef struct __attribute__((packed)){
	uint8_t packet_id;
	uint8_t length_in_bytes;
	int16_t left_flap_cmd;
	int16_t right_flap_cmd;
	int16_t left_aileron_cmd;
	int16_t right_aileron_cmd;
	int16_t left_elevator_cmd;
	int16_t right_elevator_cmd;
	int16_t left_rudder_cmd;
	int16_t right_rudder_cmd;
	int16_t throttle_cmd;
	int16_t nose_wheel_cmd;
	uint8_t brake_cmd;
} PAYLOAD_DISCO;

typedef struct __attribute__((packed)){
	uint8_t protocol_iden_hi;
	uint8_t protocol_iden_low;
	uint8_t protocol_version;
	uint8_t take_control : 1;
	uint8_t source_id : 7;
	uint8_t destination_id;
	uint8_t sequence;
	uint16_t length;
	uint32_t unix_time_stamp;
	uint32_t utc_microsecond_time_stamp;
	uint8_t header_check;
	PAYLOAD_DISCO user_data;
	uint16_t checksum;
} MESSAGE_HEADER;

extern "C" __EXPORT int u1_protocol_main(int argc, char *argv[]);
