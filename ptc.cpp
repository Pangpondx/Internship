#include "disco_protocol.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/time.h>

static int uart_open(void)
{
    int fd = open(UART_DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) {
        PX4_ERR("open %s failed (%d)", UART_DEVICE, errno);
    }

    return fd;
}

static void uart_config(int fd)
{
    struct termios t {};

    tcgetattr(fd, &t);
    cfsetispeed(&t, UART_BAUDRATE);
    cfsetospeed(&t, UART_BAUDRATE);

    t.c_cflag |= (CLOCAL | CREAD);
    t.c_cflag &= ~CSIZE;
    t.c_cflag |= CS8;
    t.c_cflag &= ~PARENB;
    t.c_cflag &= ~CSTOPB;
    t.c_cflag &= ~CRTSCTS;

    t.c_lflag = 0;
    t.c_oflag = 0;
    t.c_iflag = 0;

    tcsetattr(fd, TCSANOW, &t);
}

static void dump_hex(const uint8_t *buf, int len)
{
    char line[128];
    int idx = 0;

    for (int i = 0; i < len && idx < (int)sizeof(line) - 4; i++) {
        idx += snprintf(&line[idx], sizeof(line) - idx, "%02X ", buf[i]);
    }

    PX4_INFO("RX HEX: %s", line);
}

static void uart_test_loop(int fd)
{
    MESSAGE_HEADER pkt;
    uint8_t rx_buf[256];

    while (true) {
        int n = read(fd, rx_buf, sizeof(rx_buf));

        if (n > 0) {
            dump_hex(rx_buf, n);
	    memcpy(&pkt, rx_buf, sizeof(MESSAGE_HEADER));
	    PX4_INFO("%u %u %u %u %u %u %u %u %u %u %u %u",(unsigned int)pkt.protocol_iden_hi, (unsigned int)pkt.protocol_iden_low, (unsigned int)pkt.protocol_version, (unsigned int)pkt.take_control, (unsigned int)pkt.source_id, (unsigned int)pkt.destination_id, (unsigned int)pkt.sequence, (unsigned int)pkt.length, (unsigned int)pkt.unix_time_stamp, (unsigned int)pkt.utc_microsecond_time_stamp, (unsigned int)pkt.header_check, pkt.checksum );
	    PX4_INFO("%u %u %d %d %d %d %d %d %d %d %d %d %u",(unsigned int)pkt.user_data.packet_id, (unsigned int)pkt.user_data.length_in_bytes, pkt.user_data.left_flap_cmd, pkt.user_data.right_flap_cmd, pkt.user_data.left_aileron_cmd, pkt.user_data.right_aileron_cmd, pkt.user_data.left_elevator_cmd, pkt.user_data.right_elevator_cmd, pkt.user_data.left_rudder_cmd, pkt.user_data.right_rudder_cmd, pkt.user_data.throttle_cmd, pkt.user_data.nose_wheel_cmd, (unsigned int)pkt.user_data.brake_cmd);
        }
        px4_usleep(1000000);
    }
}

extern "C" int u1_protocol_main(int argc, char *argv[])
{
    PX4_INFO("u1 protocol test start");

    int fd = uart_open();
    if (fd < 0) {
        PX4_ERR("UART open failed");
        return -1;
    }

    uart_config(fd);
    uart_test_loop(fd);

    close(fd);
    return 0;
}
