#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>




int main(void)
{
    
    long  re = syscall(SYS_write,-1,"hello\n",6);

    if (re  == - 1)
    {
       int err = errno;
       fprintf(stderr , "Fatal error :%s\n",strerror(err));
       return 1;
    }
    return 0;
}

