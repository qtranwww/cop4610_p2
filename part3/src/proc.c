#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include "elevator.h"

static const char *state_to_string(enum elevator_state state) {
    switch (state) {
        case ELEVATOR_OFFLINE: return "OFFLINE";
        case ELEVATOR_IDLE:    return "IDLE";
        case ELEVATOR_LOADING: return "LOADING";
        case ELEVATOR_UP:      return "UP";
        case ELEVATOR_DOWN:    return "DOWN";
        default:               return "UNKNOWN";
    }
}

static int elevator_proc_show(struct seq_file *m, void *v) {
    int i;
    struct list_head *ptr;
    struct pet *temp_pet;
    
    // We don't use 'v'. cast it to void to prevent compiler warnings
    (void)v; 

    mutex_lock(&elevator_lock);

    seq_printf(m, "Elevator state: ");
    if (elevator.state == ELEVATOR_UP)      
        seq_printf(m, "UP\n");
    else if (elevator.state == ELEVATOR_DOWN) 
        seq_printf(m, "DOWN\n");
    else if (elevator.state == ELEVATOR_LOADING) 
        seq_printf(m, "LOADING\n");
    else if (elevator.state == ELEVATOR_IDLE)    
        seq_printf(m, "IDLE\n");
    else
        seq_printf(m, "OFFLINE\n");


    seq_printf(m, "Current floor: %d\n", elevator.floor);
    seq_printf(m, "Current load: %d lbs\n", elevator.load);


    seq_printf(m, "Elevator status: ");

    for (ptr = elevator.car.next; ptr != &elevator.car; ptr = ptr->next) {
        temp_pet = list_entry(ptr, struct pet, node);
        seq_printf(m, "%c%d ", pet_info[temp_pet->type].symbol, temp_pet->dest);
    }
    seq_printf(m, "\n\n");

    for (i = MAX_FLOOR; i >= MIN_FLOOR; i--) {
        struct list_head *head = floor_queue(i);
        int count = 0;


        for (ptr = head->next; ptr != head; ptr = ptr->next) {
            count = count + 1;
        }

        if (elevator.floor == i)
            seq_printf(m, "[*] ");
        else
            seq_printf(m, "[ ] ");
        
        seq_printf(m, "Floor %d: %d ", i, count);


        for (ptr = head->next; ptr != head; ptr = ptr->next) {
            temp_pet = list_entry(ptr, struct pet, node);
            seq_printf(m, "%c%d ", pet_info[temp_pet->type].symbol, temp_pet->dest);
        }
        seq_printf(m, "\n");
    }

    seq_printf(m, "\nNumber of pets: %d\n", elevator.passengers);
    seq_printf(m, "Number of pets waiting: %d\n", elevator.waiting);
    seq_printf(m, "Number of pets serviced: %d\n", elevator.serviced);

    mutex_unlock(&elevator_lock);
    return 0;
}

static int elevator_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, elevator_proc_show, NULL);
}

static const struct proc_ops elevator_proc_ops = {
    .proc_open = elevator_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

int elevator_proc_init(void) {
    if (proc_create("elevator", 0444, NULL, &elevator_proc_ops) == NULL) {
        return -ENOMEM;
    }
    return 0;
}

void elevator_proc_exit(void) {
    remove_proc_entry("elevator", NULL);
}