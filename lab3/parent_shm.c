#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
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

int main(void) {
    char filename[256];
    safe_write(STDOUT_FILENO, "Enter filename: ", 16);
    ssize_t bytes = read(STDIN_FILENO, filename, sizeof(filename) - 1);
    if (bytes <= 0) {
        _exit(EXIT_FAILURE);
    }

    filename[bytes] = '\0';
    filename[strcspn(filename, "\r\n")] = '\0';

    char shm_name[64], sem_parent_name[64], sem_child_name[64];
    snprintf(shm_name, sizeof(shm_name), "/shm_%d", getpid());
    snprintf(sem_parent_name, sizeof(sem_parent_name), "/sem_parent_%d", getpid());
    snprintf(sem_child_name, sizeof(sem_child_name), "/sem_child_%d", getpid());

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        _exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(struct shm_data)) == -1) {
        _exit(EXIT_FAILURE);
    }


    struct shm_data *data = mmap(NULL, sizeof(struct shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_parent = sem_open(sem_parent_name, O_CREAT, 0666, 0);
    sem_t *sem_child = sem_open(sem_child_name, O_CREAT, 0666, 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        _exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == 0) {
        char *const args[] = {
            "./child_shm", filename, shm_name, sem_parent_name, sem_child_name, NULL
        };

        execv("./child_shm", args);
        _exit(EXIT_FAILURE);
    }

    safe_write(STDOUT_FILENO, "Enter numbers (empty line to finish):\n", 38);

    char line[SHM_SIZE];
    while (1) {
        bytes = read(STDIN_FILENO, line, sizeof(line) - 1);
        if (bytes <= 0) {
            break;
        }

        line[bytes] = '\0';
        line[strcspn(line, "\r\n")] = '\0';

        strncpy(data->buffer, line, SHM_SIZE);
        data->buffer[SHM_SIZE - 1] = '\0';

        sem_post(sem_child);
        if (strlen(line) == 0) {
            break;
        }
        
        sem_wait(sem_parent);
    }

    wait(NULL);

    munmap(data, sizeof(struct shm_data));
    close(shm_fd);
    shm_unlink(shm_name);
    sem_close(sem_parent);
    sem_close(sem_child);
    sem_unlink(sem_parent_name);
    sem_unlink(sem_child_name);

    return EXIT_SUCCESS;
}
