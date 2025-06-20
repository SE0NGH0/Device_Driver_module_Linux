// user_read_stdio.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define DEVICE_PATH "/dev/mychardev"
#define BUF_SIZE    128

int main(void) {
    FILE *fp = fopen(DEVICE_PATH, "r");
    if (!fp) {
        fprintf(stderr, "fopen failed for %s: %s\n", DEVICE_PATH, strerror(errno));
        return EXIT_FAILURE;
    }

    char buf[BUF_SIZE];
    size_t n = fread(buf, 1, BUF_SIZE - 1, fp);
    if (n == 0 && ferror(fp)) {
        fprintf(stderr, "fread error: %s\n", strerror(errno));
        fclose(fp);
        return EXIT_FAILURE;
    }

    buf[n] = '\0';
    printf("Read %zu bytes: %s", n, buf);

    fclose(fp);
    return EXIT_SUCCESS;
}

