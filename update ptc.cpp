#include "disco_protocol.h"

#include <px4_platform_common/module.h>
#include <px4_platform_common/log.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define UART_PORT "/dev/ttyS1" // แก้ Port ตามจริงของคุณ
#define UART_BAUDRATE B115200

static const uint16_t crc16tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

int open_uart() {
    int fd = open(UART_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return -1;

    struct termios uart_config;
    tcgetattr(fd, &uart_config);
    cfsetispeed(&uart_config, UART_BAUDRATE);
    cfsetospeed(&uart_config, UART_BAUDRATE);
    uart_config.c_cflag |= (CLOCAL | CREAD);
    uart_config.c_cflag &= ~CSIZE;
    uart_config.c_cflag |= CS8;
    uart_config.c_cflag &= ~PARENB;
    uart_config.c_cflag &= ~CSTOPB;
    uart_config.c_cflag &= ~CRTSCTS;
    tcsetattr(fd, TCSANOW, &uart_config);
    return fd;
}

uint8_t cal_header_check(const uint8_t* pkt) {
    const uint8_t* ptr = (const uint8_t*)pkt;
    uint16_t sum = 0;

    for(int i=0; i<16; i++){
        sum += ptr[i];
    }
    return (uint8_t)sum;
}

uint16_t GetCRC16(const void *buf, unsigned len) {
    uint16_t crc;
    const uint8_t *bf;
    bf = (const uint8_t *) buf;
    crc = 0xffff;
    do {
        crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *bf++) & 0xFF];
    } while (--len);
    return ~crc;
}

extern "C" __EXPORT int u1_protocol_main(int argc, char *argv[])
{
    printf("Starting Disco Protocol Test Loop...\n");

    int fd = open_uart();
    if (fd < 0) {
        printf("Error: Cannot open UART\n");
        return -1;
    }

    uint8_t buffer[42];

    while (true) {

        int n = read(fd, buffer, sizeof(buffer));

        if (n == sizeof(buffer)) {

            MESSAGE_HEADER* msg = (MESSAGE_HEADER*)buffer;

            uint16_t calced_crc = GetCRC16(buffer, 40);
            if (calced_crc != msg->checksum) {
                continue;
            }
            if (msg->protocol_iden_hi != 0x5A || msg->protocol_iden_low != 0xA5) {
                continue;
            }
            if (cal_header_check(buffer) != msg->header_check) {
                continue;
            }

            if (msg->user_data.packet_id != 0x02) {
                continue;
            }
	    MESSAGE_HEADER message;
	    memcpy(&message, buffer, sizeof(MESSAGE_HEADER));

        //     float left_flap_pct = (msg->user_data.left_flap_cmd / 32767.0f) * 140.0f;
        //     float brake_pct     = (msg->user_data.brake_cmd / 200.0f) * 100.0f;

        //     printf("[VALID] L_Flap: %.2f%%, Brake: %.2f%%\n",
        //            (double)left_flap_pct, (double)brake_pct);
	    PX4_INFO("%u %u %u %u %u %u %u %u %u %u %u %u",(unsigned int)message.protocol_iden_hi, (unsigned int)message.protocol_iden_low, (unsigned int)message.protocol_version, (unsigned int)message.take_control, (unsigned int)message.source_id, (unsigned int)message.destination_id, (unsigned int)message.sequence, (unsigned int)message.length, (unsigned int)message.unix_time_stamp, (unsigned int)message.utc_microsecond_time_stamp, (unsigned int)message.header_check, message.checksum );
	    PX4_INFO("%u %u %d %d %d %d %d %d %d %d %d %d %u",(unsigned int)message.user_data.packet_id, (unsigned int)message.user_data.length_in_bytes, message.user_data.left_flap_cmd, message.user_data.right_flap_cmd, message.user_data.left_aileron_cmd, message.user_data.right_aileron_cmd, message.user_data.left_elevator_cmd, message.user_data.right_elevator_cmd, message.user_data.left_rudder_cmd, message.user_data.right_rudder_cmd, message.user_data.throttle_cmd, message.user_data.nose_wheel_cmd, (unsigned int)message.user_data.brake_cmd);

        } else {
            usleep(10000);
        }
    }

    close(fd);
    return 0;
}
