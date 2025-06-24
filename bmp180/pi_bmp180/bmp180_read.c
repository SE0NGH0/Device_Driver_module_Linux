// bmp180_read.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define I2C_DEV_DIR   "/sys/bus/i2c/devices"
#define TEMP_FILE     "temp"
#define PRESS_FILE    "pressure"

// 디렉토리 안에 temp와 pressure 파일이 모두 있는 첫 번째 디렉토리 검색
static int find_bmp180_dir(char *out, size_t len) {
    DIR *d = opendir(I2C_DEV_DIR);
    struct dirent *ent;
    if (!d) return -1;

    while ((ent = readdir(d)) != NULL) {
        // 디렉토리 이름이 "버스-주소" 형태인지 간단히 체크
        if (strchr(ent->d_name, '-') == NULL) 
            continue;

        char path_temp[256], path_press[256];
        snprintf(path_temp,  sizeof(path_temp),
                 I2C_DEV_DIR "/%s/" TEMP_FILE, ent->d_name);
        snprintf(path_press, sizeof(path_press),
                 I2C_DEV_DIR "/%s/" PRESS_FILE, ent->d_name);

        if (access(path_temp, R_OK) == 0 && access(path_press, R_OK) == 0) {
            snprintf(out, len, I2C_DEV_DIR "/%s", ent->d_name);
            closedir(d);
            return 0;
        }
    }

    closedir(d);
    return -1;
}

// sysfs에서 정수 읽기
static int read_int(const char *path, int *out) {
    char buf[32];
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    close(fd);
    if (n <= 0) return -1;
    buf[n] = '\0';
    *out = atoi(buf);
    return 0;
}

int main(void) {
    char devdir[128];
    if (find_bmp180_dir(devdir, sizeof(devdir)) < 0) {
        fprintf(stderr, "ERROR: bmp180 모듈 디렉토리를 찾을 수 없습니다.\n");
        return 1;
    }

    char temp_path[256], press_path[256];
    snprintf(temp_path,  sizeof(temp_path),
             "%s/" TEMP_FILE,  devdir);
    snprintf(press_path, sizeof(press_path),
             "%s/" PRESS_FILE, devdir);

    printf("BMP180 기압·온도 측정 (Ctrl+C로 종료)\n\n");
    while (1) {
        int t_raw, p_raw;
        if (read_int(temp_path,  &t_raw) < 0 ||
            read_int(press_path, &p_raw) < 0) {
            fprintf(stderr,
                "센서 값 읽기 실패: %s\n",
                strerror(errno));
            return 1;
        }

        // 모듈에서 temp는 0.1°C 단위로, pressure는 Pa 단위로 리턴됩니다.
        double temp_c = t_raw / 10.0;
        double press_hpa = p_raw / 100.0;

        printf("온도: %.1f °C    압력: %.2f hPa\n",
               temp_c, press_hpa);
        sleep(1);
    }
    return 0;
}


