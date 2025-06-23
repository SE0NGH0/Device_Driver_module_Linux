#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
        int fd = open("/dev/mychardev", O_RDONLY);
        if (fd < 0 )
        {
                perror("open");
                return 1;
        }
        char buf[128];
        ssize_t n = read(fd,buf,sizeof(buf)-1);
        if (n > 0)
        {
                buf[n] = '\0';
                printf("Read from device: %s\n",buf);
        }
        else
        {
                perror("read");
        }
 
        close(fd);
        return 0;
} 

