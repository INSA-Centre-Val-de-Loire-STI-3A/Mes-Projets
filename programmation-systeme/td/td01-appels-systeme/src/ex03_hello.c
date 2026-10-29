#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

void my_perror(const char *message)
{
    int err = errno;

    fprintf(stderr, "%s: %s\n", message, strerror(err));
}

int main(void)
{
    long re = syscall(SYS_write, -1, "hello\n", 6);

    if (re == -1)
    {
        my_perror("Fatal error");
        return 1;
    }

    return 0;
}

