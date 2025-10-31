#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

static void safe_write(int fd, const char *buf, size_t len) {
    ssize_t res = write(fd, buf, len);
    if (res < 0 || (size_t)res != len) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    char filename[256];
    
    safe_write(STDOUT_FILENO, "Enter filename: ", 16);
    
    ssize_t bytes = read(STDIN_FILENO, filename, sizeof(filename) - 1);
    if (bytes <= 0) {
        const char msg[] = "error: failed to read filename\n";
        safe_write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    filename[bytes] = '\0';
    
    filename[strcspn(filename, "\r\n")] = '\0';

    {
        const char msg[] = "Enter numbers (empty line to finish):\n";
        safe_write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    }

    char line[MAX_LINE_LENGTH];

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child == 0) {
        close(pipefd[1]);                 
        dup2(pipefd[0], STDIN_FILENO);    
        close(pipefd[0]);

        char *const args[] = {"./child", filename, NULL};
        execv("./child", args);

        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[0]); 

        while ((bytes = read(STDIN_FILENO, line, sizeof(line) - 1)) > 0) {
            line[bytes] = '\0';
            line[strcspn(line, "\r\n")] = '\0';

            if (strlen(line) == 0) {
                break;
            }

            safe_write(pipefd[1], line, strlen(line));
            safe_write(pipefd[1], "\n", 1);
        }

        close(pipefd[1]);
        wait(NULL);
    }

    return EXIT_SUCCESS;
}
