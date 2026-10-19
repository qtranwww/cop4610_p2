#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("COP4610");
MODULE_DESCRIPTION("Reports the current time and elapsed time through /proc/timer");
MODULE_VERSION("1.0");

#define PROC_ENTRY_NAME "timer"
#define PROC_PERMISSIONS 0444
#define TIMER_BUFFER_SIZE 256

static struct proc_dir_entry *timer_entry;
static DEFINE_MUTEX(timer_lock);
static struct timespec64 previous_time;
static bool has_previous_time;

static ssize_t timer_read(struct file *file,
                          char __user *user_buffer,
                          size_t count,
                          loff_t *position)
{
    struct timespec64 current_time;
    struct timespec64 elapsed_time;
    char buffer[TIMER_BUFFER_SIZE];
    int length;
    int lock_result;

    (void)file;

    if (*position != 0 || count == 0)
        return 0;

    lock_result = mutex_lock_interruptible(&timer_lock);
    if (lock_result != 0)
        return lock_result;

    ktime_get_real_ts64(&current_time);

    length = scnprintf(buffer,
                       sizeof(buffer),
                       "current time: %lld.%09ld\n",
                       (long long)current_time.tv_sec,
                       current_time.tv_nsec);

    if (has_previous_time) {
        elapsed_time = timespec64_sub(current_time, previous_time);
        length += scnprintf(buffer + length,
                            sizeof(buffer) - length,
                            "elapsed time: %lld.%09ld\n",
                            (long long)elapsed_time.tv_sec,
                            elapsed_time.tv_nsec);
    }

    previous_time = current_time;
    has_previous_time = true;

    mutex_unlock(&timer_lock);

    return simple_read_from_buffer(user_buffer,
                                   count,
                                   position,
                                   buffer,
                                   length);
}

static const struct proc_ops timer_proc_ops = {
    .proc_read = timer_read,
};

static int __init my_timer_init(void)
{
    timer_entry = proc_create(PROC_ENTRY_NAME,
                              PROC_PERMISSIONS,
                              NULL,
                              &timer_proc_ops);
    if (timer_entry == NULL)
        return -ENOMEM;

    has_previous_time = false;
    return 0;
}

static void __exit my_timer_exit(void)
{
    proc_remove(timer_entry);
}

module_init(my_timer_init);
module_exit(my_timer_exit);
