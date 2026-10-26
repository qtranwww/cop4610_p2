#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/wait.h>

#include "elevator.h"

#define LOAD_DELAY_MS 1000
#define MOVE_DELAY_MS 2000

static struct task_struct *elevator_task;
static DECLARE_WAIT_QUEUE_HEAD(elevator_wq);

/*
 * Bumped under elevator_lock whenever something could give the thread work.
 * An idle thread sleeps until it changes, so no wakeup can be missed between
 * dropping the lock and going to sleep.
 */
static unsigned int elevator_events;

void elevator_notify(void)
{
	WRITE_ONCE(elevator_events, elevator_events + 1);
	wake_up_interruptible(&elevator_wq);
}

static enum elevator_state choose_state(void)
{
	if (elevator.state == ELEVATOR_OFFLINE)
		return ELEVATOR_OFFLINE;

	if (pets_to_unload() || (!elevator.stopping && pets_to_load()))
		return ELEVATOR_LOADING;

	if (elevator.stopping && list_empty(&elevator.car)) {
		elevator.stopping = false;
		return ELEVATOR_OFFLINE;
	}

	return elevator_next_move();
}

static int elevator_run(void *unused)
{
	enum elevator_state next;
	unsigned int seen;

	while (!kthread_should_stop()) {
		mutex_lock(&elevator_lock);
		next = choose_state();
		elevator.state = next;
		seen = elevator_events;
		mutex_unlock(&elevator_lock);

		/* Sleep with the lock dropped so syscalls and /proc never stall. */
		switch (next) {
		case ELEVATOR_LOADING:
			msleep(LOAD_DELAY_MS);
			mutex_lock(&elevator_lock);
			unload_pets();
			if (!elevator.stopping)
				load_pets();
			mutex_unlock(&elevator_lock);
			break;
		case ELEVATOR_UP:
		case ELEVATOR_DOWN:
			msleep(MOVE_DELAY_MS);
			mutex_lock(&elevator_lock);
			elevator.floor += next == ELEVATOR_UP ? 1 : -1;
			mutex_unlock(&elevator_lock);
			break;
		default:
			wait_event_interruptible(elevator_wq,
						 kthread_should_stop() ||
						 READ_ONCE(elevator_events) != seen);
		}
	}

	return 0;
}

int elevator_thread_start(void)
{
	elevator_task = kthread_run(elevator_run, NULL, "elevator");
	return PTR_ERR_OR_ZERO(elevator_task);
}

void elevator_thread_stop(void)
{
	kthread_stop(elevator_task);
}

int elevator_start(void)
{
	int ret = 0;

	if (mutex_lock_interruptible(&elevator_lock))
		return -EINTR;

	if (elevator.state != ELEVATOR_OFFLINE) {
		ret = 1;
	} else {
		elevator.state = ELEVATOR_IDLE;
		elevator_notify();
	}

	mutex_unlock(&elevator_lock);
	return ret;
}

int elevator_stop(void)
{
	int ret = 0;

	if (mutex_lock_interruptible(&elevator_lock))
		return -EINTR;

	if (elevator.stopping) {
		ret = 1;
	} else if (elevator.state != ELEVATOR_OFFLINE) {
		elevator.stopping = true;
		elevator_notify();
	}

	mutex_unlock(&elevator_lock);
	return ret;
}
