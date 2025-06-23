#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd;
    if (argc != 2 || (argv[1][0] != '0' && argv[1][0] != '1')) {
        printf("사용법: %s 0|1\n", argv[0]);
        return 1;
    }

    fd = open("/dev/led-driver", O_WRONLY);
    if (fd < 0) {
        perror("디바이스 열기 실패");
        return 1;
    }

    write(fd, argv[1], 1);
    close(fd);
    return 0;
}

