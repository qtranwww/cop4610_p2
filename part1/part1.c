#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    (void)syscall(SYS_getpid);
    (void)syscall(SYS_getppid);
    (void)syscall(SYS_getuid);
    (void)syscall(SYS_getgid);
    (void)syscall(SYS_geteuid);

    return 0;
}
