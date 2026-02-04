#include "disco_protocol.h"
// #include <px4_platform_common/getopt.h>

DiscoProtocol *g_disco_instance = nullptr;

DiscoProtocol::DiscoProtocol() : ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::hp_default)
{
	sem_init(&process_sem, 0, 0);
	pthread_mutex_init(&data_mutex, nullptr);
}

DiscoProtocol::~DiscoProtocol()
{
	task_should_exit = true;
	sem_post(&process_sem);
	if(fd >= 0){
		::close(fd);
		fd = -1;
	}
	sem_destroy(&process_sem);
	pthread_mutex_destroy(&data_mutex);
}

bool DiscoProtocol::Init()
{
	PX4_INFO("Init start");
	task_should_exit = false;
	process_task_id = px4_task_spawn_cmd("disco_process",
		                             SCHED_DEFAULT,
					     SCHED_PRIORITY_DEFAULT,
					     2048,
					     (px4_main_t)&DiscoProtocol::process_task_tampoline,
					     nullptr);
	read_task_id    = px4_task_spawn_cmd("disco_read",
		                             SCHED_DEFAULT,
					     SCHED_PRIORITY_DEFAULT,
					     2048,
					     (px4_main_t)&DiscoProtocol::read_serial_task_tampoline,
					     nullptr);
	if(process_task_id < 0 || read_task_id < 0){
		PX4_ERR("Failed to spawn task");
		return false;
	}
	PX4_INFO("SPAWN");
	this->ScheduleOnInterval(20000);
	this->ScheduleNow();
	PX4_INFO("COMPLETE");
	return true;
}

//-----main loop-----//
void DiscoProtocol::Run()
{
	if(should_exit()){
		ScheduleClear();
		task_should_exit = true;
		sem_post(&process_sem);
		exit_and_cleanup();
		return;
	}
	pthread_mutex_lock(&data_mutex);
	disco_msg.success = success;
	disco_msg.error = error;
	disco_msg.timestamp = hrt_absolute_time();
	disco_pub.publish(disco_msg);
	pthread_mutex_unlock(&data_mutex);
}

// DiscoProtocol *DiscoProtocol::instantiate(int argc, char *argv[])
// {
// 	g_disco_instance = new DiscoProtocol();
// 	return g_disco_instance;
// }

int DiscoProtocol::task_spawn(int argc, char *argv[])
{
	DiscoProtocol *instance = new DiscoProtocol();
	if(instance){
		g_disco_instance = instance;
		_object.store(instance);
		_task_id = task_id_is_work_queue;
		if(instance->Init()){
			PX4_INFO("OK");
			return PX4_OK;
		}else{
			g_disco_instance = nullptr;
			_object.store(nullptr);
			delete instance;
			return PX4_ERROR;
		}
	}
	PX4_ERR("Error");
	return PX4_ERROR;
}

int DiscoProtocol::print_usage(const char *reason)
{
	if(reason){
		PX4_WARN("%s\n", reason);
	}
	PRINT_MODULE_DESCRIPTION("Disco Protocol Driver handles UART communication");
	PRINT_MODULE_USAGE_NAME("disco_protocol", "driver");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();
	return 0;
}

int DiscoProtocol::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}
extern "C" __EXPORT int u1_protocol_main(int argc, char *argv[])
{
	return DiscoProtocol::main(argc, argv);
}
.
