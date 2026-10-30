#include <errno.h>
#include <sys/syscall.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
void my_perror(const char *message)
{
    int code = errno;
    fprintf(stderr,  "%s : %s \n", message,strerror(code));
}

int main(void)
{
    long ret = syscall(SYS_write, 9, "hello\n",6); /* fd volontairement invalide (exercice 2) */
    if(ret == -1)
    {
        my_perror("write");
        return 1;
    }

    return 0;
}
