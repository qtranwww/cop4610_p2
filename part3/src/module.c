#include <linux/init.h>
#include <linux/module.h>

#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pet elevator scheduler");

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int start_floor, int destination_floor, int type);
extern int (*STUB_stop_elevator)(void);

static int __init elevator_init(void)
{
	int ret = elevator_thread_start();

	if (ret)
		return ret;

	STUB_start_elevator = elevator_start;
	STUB_issue_request = elevator_request;
	STUB_stop_elevator = elevator_stop;
	return 0;
}

/* Unhook the syscalls before the thread and pets they touch go away. */
static void __exit elevator_exit(void)
{
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;

	elevator_thread_stop();
	free_all_pets();
}

module_init(elevator_init);
module_exit(elevator_exit);
