#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define SHM_SIZE 1024

struct shm_data {
    char buffer[SHM_SIZE];
};

static void safe_write(int fd, const char *buf, size_t len) {
    ssize_t res = write(fd, buf, len);
    if (res < 0 || (size_t)res != len) {
        _exit(EXIT_FAILURE);
    }
}

double parse_double(const char *str, char **end) {
    double val = strtod(str, end);
    return val;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        _exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    const char *shm_name = argv[2];
    const char *sem_parent_name = argv[3];
    const char *sem_child_name = argv[4];

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        _exit(EXIT_FAILURE);
    }

    struct shm_data *data = mmap(NULL, sizeof(struct shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_parent = sem_open(sem_parent_name, 0);
    sem_t *sem_child = sem_open(sem_child_name, 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        _exit(EXIT_FAILURE);
    }

    int file_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (file_fd == -1) {
        _exit(EXIT_FAILURE);
    }

    while (1) {
        sem_wait(sem_child);

        if (strlen(data->buffer) == 0) {
            break;
        }

        char *ptr = data->buffer;
        double sum = 0.0;
        while (*ptr) {
            double num = strtod(ptr, &ptr);
            sum += num;
        }

        char result[64];
        int len = snprintf(result, sizeof(result), "%g\n", sum);
        if (len < 0 || len >= (int)sizeof(result)) {
            _exit(EXIT_FAILURE);
        }

        safe_write(file_fd, result, (size_t)len);
        sem_post(sem_parent);
    }

    close(file_fd);
    munmap(data, sizeof(struct shm_data));
    close(shm_fd);

    return EXIT_SUCCESS;
}
