#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) {
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

typedef struct __attribute__((packed)) {
    uint8_t protocol_iden_hi;
    uint8_t protocol_iden_low;
    uint8_t protocol_version;
    uint8_t source_id : 7;
    uint8_t take_control : 1;
    uint8_t destination_id;
    uint8_t sequence;
    uint16_t length;
    uint32_t unix_time_stamp;
    uint32_t utc_microsecond_time_stamp;
    uint8_t header_check;
    PAYLOAD_DISCO user_data;
    uint16_t checksum;
} MESSAGE_HEADER;



