#include <sys/syscall.h>
#include <unistd.h>
void main(void)
{
    (void)syscall(SYS_write, STDOUT_FILENO, "hello\n",6);
}