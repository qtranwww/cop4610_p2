#include <linux/errno.h>
#include <linux/slab.h>

#include "elevator.h"

const struct pet_info pet_info[NUM_PET_TYPES] = {
	[PET_CHIHUAHUA] = { 'C', 3 },
	[PET_PUG]       = { 'P', 14 },
	[PET_PUGHUAHUA] = { 'H', 10 },
	[PET_DACHSHUND] = { 'D', 16 },
};

DEFINE_MUTEX(elevator_lock);

struct elevator elevator = {
	.state = ELEVATOR_OFFLINE,
	.floor = MIN_FLOOR,
	.car = LIST_HEAD_INIT(elevator.car),
	.floors = {
		LIST_HEAD_INIT(elevator.floors[0]),
		LIST_HEAD_INIT(elevator.floors[1]),
		LIST_HEAD_INIT(elevator.floors[2]),
		LIST_HEAD_INIT(elevator.floors[3]),
		LIST_HEAD_INIT(elevator.floors[4]),
	},
};

static bool valid_floor(int floor)
{
	return floor >= MIN_FLOOR && floor <= MAX_FLOOR;
}

struct list_head *floor_queue(int floor)
{
	return &elevator.floors[floor - MIN_FLOOR];
}

static bool pet_fits(const struct pet *pet)
{
	return elevator.passengers < MAX_PETS &&
	       elevator.load + pet_info[pet->type].weight <= MAX_WEIGHT;
}

int elevator_request(int start, int dest, int type)
{
	struct pet *pet;

	if (!valid_floor(start) || !valid_floor(dest) || start == dest ||
	    type < 0 || type >= NUM_PET_TYPES)
		return 1;

	pet = kmalloc(sizeof(*pet), GFP_KERNEL);
	if (!pet)
		return -ENOMEM;

	pet->type = type;
	pet->dest = dest;

	if (mutex_lock_interruptible(&elevator_lock)) {
		kfree(pet);
		return -EINTR;
	}
	list_add_tail(&pet->node, floor_queue(start));
	elevator.waiting++;
	elevator_notify();
	mutex_unlock(&elevator_lock);

	return 0;
}

bool pets_to_unload(void)
{
	struct pet *pet;

	list_for_each_entry(pet, &elevator.car, node)
		if (pet->dest == elevator.floor)
			return true;
	return false;
}

/* Strict FIFO: a pet that doesn't fit blocks everyone queued behind it. */
bool pets_to_load(void)
{
	struct list_head *queue = floor_queue(elevator.floor);

	return !list_empty(queue) &&
	       pet_fits(list_first_entry(queue, struct pet, node));
}

void unload_pets(void)
{
	struct pet *pet, *next;

	list_for_each_entry_safe(pet, next, &elevator.car, node) {
		if (pet->dest != elevator.floor)
			continue;
		list_del(&pet->node);
		elevator.load -= pet_info[pet->type].weight;
		elevator.passengers--;
		elevator.serviced++;
		kfree(pet);
	}
}

void load_pets(void)
{
	struct list_head *queue = floor_queue(elevator.floor);
	struct pet *pet;

	while (pets_to_load()) {
		pet = list_first_entry(queue, struct pet, node);
		list_move_tail(&pet->node, &elevator.car);
		elevator.load += pet_info[pet->type].weight;
		elevator.passengers++;
		elevator.waiting--;
	}
}

static void free_list(struct list_head *list)
{
	struct pet *pet, *next;

	list_for_each_entry_safe(pet, next, list, node) {
		list_del(&pet->node);
		kfree(pet);
	}
}

void free_all_pets(void)
{
	int i;

	free_list(&elevator.car);
	for (i = 0; i < NUM_FLOORS; i++)
		free_list(&elevator.floors[i]);
}
