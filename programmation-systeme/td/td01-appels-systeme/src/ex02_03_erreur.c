#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
int main(void)
{
    long ret = syscall(SYS_write, 9, "hello\n",6); /* fd volontairement invalide (exercice 2) */
    if(ret == -1)
    {
        perror("write");
        return 1;
    }

    return 0;
}
