// bh1750_read.c (using sysfs interface, continuous measurement)
// 유저스페이스에서 sysfs로 BH1750 조도 센서 값을 계속 읽어 출력

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    const char *sysfs_path = "/dev/bh1750";
    char buf[32];

    while (1) {
        FILE *fp = fopen(sysfs_path, "r");
        if (!fp) {
            perror("fopen sysfs lux");
            return EXIT_FAILURE;
        }

        if (fgets(buf, sizeof(buf), fp) != NULL) {
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n')
                buf[len-1] = '\0';
            printf("Ambient Light: %s lux\n", buf);
        } else {
            fprintf(stderr, "Failed to read lux value from %s\n", sysfs_path);
            fclose(fp);
            return EXIT_FAILURE;
        }

        fclose(fp);
        sleep(1); // 1초 간격으로 측정
    }

    return EXIT_SUCCESS;
}

