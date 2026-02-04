#include "disco_protocol.h"

extern DiscoProtocol *g_disco_instance;

int DiscoProtocol::process_task_tampoline(int argc, char *argv[])
{
	if(g_disco_instance){
		g_disco_instance->process_task_main();
	}
	return 0;
}

float DiscoProtocol::convert_raw_to_percent(int16_t raw_value)
{
	//////raw data 32767 = 140%//////
            //32767 / 140 = 234.05//
	const float divisor = 234.05f;
	return raw_value / divisor;
}

void DiscoProtocol::process_task_main()
{
	while(!task_should_exit){
		sem_wait(&process_sem);
		pthread_mutex_lock(&data_mutex);
		MESSAGE_HEADER *header = (MESSAGE_HEADER*) buffer;
		disco_msg.source_id = header->source_id;
		disco_msg.take_control = header->take_control;
		disco_msg.sequence = header->sequence;
		disco_msg.unix_time_stamp = header->unix_time_stamp;
		disco_msg.utc_microsecond_time_stamp = header->utc_microsecond_time_stamp;
		uint8_t *ptr = &buffer[17];
		int count_index = 0;
		int total_len = header->length;
		while(count_index < total_len){
			uint8_t id = ptr[0];
			uint8_t len = ptr[1];
			if(id == 0x02){
				UPLINK_DISCO *data = (UPLINK_DISCO*)(ptr);
				disco_msg.packet_id = data->packet_id;
    				disco_msg.length_in_bytes = data->length_in_bytes;
    				disco_msg.left_flap_cmd = data->left_flap_cmd;
    				disco_msg.right_flap_cmd = data->right_flap_cmd;
    				disco_msg.left_aileron_cmd = data->left_aileron_cmd;
    				disco_msg.right_aileron_cmd = data->right_aileron_cmd;
    				disco_msg.left_elevator_cmd = data->left_elevator_cmd;
    				disco_msg.right_elevator_cmd = data->right_elevator_cmd;
    				disco_msg.left_rudder_cmd = data->left_rudder_cmd;
    				disco_msg.right_rudder_cmd = data->right_rudder_cmd;
    				disco_msg.throttle_cmd = data->throttle_cmd;;
    				disco_msg.nose_wheel_cmd = data->nose_wheel_cmd;
    				disco_msg.brake_cmd = data->brake_cmd;

				disco_msg.packet_id_percent = data->packet_id;
    				disco_msg.length_in_bytes_percent = data->length_in_bytes;
    				disco_msg.left_flap_cmd_percent = convert_raw_to_percent(data->left_flap_cmd);
    				disco_msg.right_flap_cmd_percent = convert_raw_to_percent(data->right_flap_cmd);
    				disco_msg.left_aileron_cmd_percent = convert_raw_to_percent(data->left_aileron_cmd);
    				disco_msg.right_aileron_cmd_percent = convert_raw_to_percent(data->right_aileron_cmd);
    				disco_msg.left_elevator_cmd_percent = convert_raw_to_percent(data->left_elevator_cmd);
    				disco_msg.right_elevator_cmd_percent = convert_raw_to_percent(data->right_elevator_cmd);
    				disco_msg.left_rudder_cmd_percent = convert_raw_to_percent(data->left_rudder_cmd);
    				disco_msg.right_rudder_cmd_percent = convert_raw_to_percent(data->right_rudder_cmd);
    				disco_msg.throttle_cmd_percent = convert_raw_to_percent(data->throttle_cmd);
    				disco_msg.nose_wheel_cmd_percent = convert_raw_to_percent(data->nose_wheel_cmd);
    				disco_msg.brake_cmd_percent = (float)data->brake_cmd / 2.0f;
			}
			int step = 2 + len;
			count_index += step;
			ptr += step;
		}
		pthread_mutex_unlock(&data_mutex);
	}
}
