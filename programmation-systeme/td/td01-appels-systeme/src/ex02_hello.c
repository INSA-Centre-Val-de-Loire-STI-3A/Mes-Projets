#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


int main(void)
{
    
    int ret = syscall(SYS_write, 3, "hello\n",6);

    if (ret  == - 1)
    {
        perror("Fatal error");
        return 1;
    }
    return 0;
}

