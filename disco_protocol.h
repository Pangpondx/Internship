#pragma once

#include <px4_platform_common/log.h>
#include <px4_platform_common/module.h>
//#include <px4_platform_common/module_params.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>
#include <uORB/Publication.hpp>
#include <uORB/topics/disco_data.h>
#include <pthread.h>
#include <semaphore.h>
#include <px4_platform_common/getopt.h>

#define UART_PORT "/dev/ttyS1"
#define UART_BAUDRATE B115200
#define MAX_PACKET_SIZE 512

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
} UPLINK_DISCO;

typedef struct __attribute__((packed)) {
    uint8_t packet_id;
    uint8_t length_in_bytes;
    float left_flap_cmd;
    float right_flap_cmd;
    float left_aileron_cmd;
    float right_aileron_cmd;
    float left_elevator_cmd;
    float right_elevator_cmd;
    float left_rudder_cmd;
    float right_rudder_cmd;
    float throttle_cmd;
    float nose_wheel_cmd;
    float brake_cmd;
} UPLINK_DISCO_PERCENT;

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
    //uint8_t header_check;
    //uint16_t checksum;
} MESSAGE_HEADER;

class DiscoProtocol : public ModuleBase<DiscoProtocol>, public px4::ScheduledWorkItem
{
public:
	DiscoProtocol();
	~DiscoProtocol() override;

	// DiscoProtocol(): ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::lp_default)
	// {
	// sem_init(&process_sem, 0, 0);
	// pthread_mutex_init(&data_mutex, nullptr);
	// };

	static DiscoProtocol *instantiate(int argc,char *argv[]);
	static int task_spawn(int argc, char *argv[]);
	static int custom_command(int argc, char *argv[]);
	static int print_usage(const char *reason = nullptr);

	bool Init();
	void Run() override;

private:
	int open_uart();
	uint8_t cal_header_check(const uint8_t* pkt);
	uint16_t GetCRC16(const void *buf, unsigned len);

	static int read_serial_task_tampoline(int argc, char *argv[]);
	void read_serial_task_main();

	static int process_task_tampoline(int argc, char *argv[]);
	void process_task_main();
	float convert_raw_to_percent(int16_t raw_value);

	enum class State{ WAIT_IDEN, HEADER_CHECKSUM, CHECK_CRC16};
	State state{State::WAIT_IDEN};

	int fd{-1};

	uint8_t buffer[MAX_PACKET_SIZE];
	uint16_t buffer_index{0};
	uint16_t payload_len{0};

	uint32_t success{0};
    	uint32_t error{0};

	sem_t process_sem;
	pthread_mutex_t data_mutex;
	int read_task_id{-1};
	int process_task_id{-1};
	bool task_should_exit{false};

	MESSAGE_HEADER last_message;

	disco_data_s disco_msg{};
	uORB::Publication<disco_data_s> disco_pub{ORB_ID(disco_data)};

	static const uint16_t crc16tab[256];
};
