#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

static void safe_write(int fd, const char *buf, size_t len) {
    ssize_t res = write(fd, buf, len);
    if (res < 0 || (size_t)res != len) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

double parse_double(const char* str, char** end) {
    double result = 0.0;
    double sign = 1.0;
    double fraction = 0.1;

    while (*str == ' ') str++;

    if (*str == '-') {
        sign = -1.0;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }

    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            result += (*str - '0') * fraction;
            fraction *= 0.1;
            str++;
        }
    }

    while (*str == ' ') str++;

    if (end) *end = (char*)str;
    return result * sign;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        const char msg[] = "usage: ./child filename\n";
        safe_write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (file == -1) {
        const char msg[] = "error: failed to open requested file\n";
        safe_write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char buf[BUFFER_SIZE];
    ssize_t bytes;

    while ((bytes = read(STDIN_FILENO, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';

        char *line = strtok(buf, "\n");
        while (line) {
            double sum = 0.0;
            char *ptr = line;

            while (*ptr) {
                double num = parse_double(ptr, &ptr);
                sum += num;
                while (*ptr == ' ') ptr++;
            }

            char result[64];
            int len = snprintf(result, sizeof(result), "%g\n", sum);
            if (len < 0 || len >= (int)sizeof(result)) {
                const char msg[] = "error: snprintf failed\n";
                safe_write(STDERR_FILENO, msg, sizeof(msg) - 1);
                exit(EXIT_FAILURE);
            }

            safe_write(file, result, (size_t)len);

            line = strtok(NULL, "\n");
        }
    }

    if (bytes < 0) {
        const char msg[] = "error: failed to read from stdin\n";
        safe_write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    close(file);
    return EXIT_SUCCESS;
}
