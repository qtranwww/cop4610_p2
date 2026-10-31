#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>

#define MIN_FLOOR 1
#define MAX_FLOOR 5
#define NUM_FLOORS (MAX_FLOOR - MIN_FLOOR + 1)
#define MAX_PETS 5
#define MAX_WEIGHT 50

enum elevator_state {
	ELEVATOR_OFFLINE,
	ELEVATOR_IDLE,
	ELEVATOR_LOADING,
	ELEVATOR_UP,
	ELEVATOR_DOWN,
};

enum pet_type {
	PET_CHIHUAHUA,
	PET_PUG,
	PET_PUGHUAHUA,
	PET_DACHSHUND,
	// NUM_PET_TYPES
};

struct pet_info {
	char symbol;
	int weight;
};

struct pet {
	enum pet_type type;
	int dest;
	struct list_head node;
};

struct elevator {
	enum elevator_state state;
	bool stopping;
	int floor;
	int load;
	int passengers;
	int waiting;
	int serviced;
	struct list_head car;
	struct list_head floors[NUM_FLOORS];
};

extern const struct pet_info pet_info[NUM_PET_TYPES];
extern struct elevator elevator;
extern struct mutex elevator_lock;

/* Syscall bodies; each takes elevator_lock itself. */
int elevator_start(void);
int elevator_request(int start, int dest, int type);
int elevator_stop(void);

int elevator_thread_start(void);
void elevator_thread_stop(void);

/* Caller must hold elevator_lock. */
struct list_head *floor_queue(int floor);
bool pets_to_unload(void);
bool pets_to_load(void);
void unload_pets(void);
void load_pets(void);
void elevator_notify(void);

/* Only safe once the elevator thread has stopped. */
void free_all_pets(void);

/*
 * Scheduler (part 3f). Called with elevator_lock held, after loading at the
 * current floor is done. Returns ELEVATOR_UP, ELEVATOR_DOWN, or ELEVATOR_IDLE.
 * Must never move past MIN_FLOOR/MAX_FLOOR, and must ignore waiting pets
 * while elevator.stopping is set.
 */
enum elevator_state elevator_next_move(void);

#endif
