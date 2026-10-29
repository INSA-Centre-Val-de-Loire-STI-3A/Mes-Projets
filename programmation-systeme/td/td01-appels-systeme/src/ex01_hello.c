#include <sys/syscall.h>
#include <unistd.h>

int main()
{
    (void) syscall(SYS_write, STDOUT_FILENO, "hello\n",6);
}


