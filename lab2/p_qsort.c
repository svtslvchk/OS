#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>     // write()
#include <string.h>     // strlen()
#include <stdio.h>      // snprintf()

typedef struct {
    int *arr;
    int left;
    int right;
} Task;

sem_t sem;

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

void quicksort(int *arr, int left, int right) {
    if (left >= right)
        return;

    int pivot = arr[(left + right) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) swap(&arr[i++], &arr[j--]);
    }

    if (left < j) quicksort(arr, left, j);
    if (i < right) quicksort(arr, i, right);
}

void *parallel_qsort(void *arg) {
    Task *task = (Task *)arg;
    int *arr = task->arr;
    int left = task->left;
    int right = task->right;
    free(task);

    if (left >= right) {
        sem_post(&sem);
        pthread_exit(NULL);
    }

    int pivot = arr[(left + right) / 2];
    int i = left, j = right;

    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) swap(&arr[i++], &arr[j--]);
    }

    pthread_t t1, t2;
    int created1 = 0, created2 = 0;

    if (sem_trywait(&sem) == 0) {
        Task *leftTask = malloc(sizeof(Task));
        leftTask->arr = arr;
        leftTask->left = left;
        leftTask->right = j;
        pthread_create(&t1, NULL, parallel_qsort, leftTask);
        created1 = 1;
    } else {
        quicksort(arr, left, j);
    }

    if (sem_trywait(&sem) == 0) {
        Task *rightTask = malloc(sizeof(Task));
        rightTask->arr = arr;
        rightTask->left = i;
        rightTask->right = right;
        pthread_create(&t2, NULL, parallel_qsort, rightTask);
        created2 = 1;
    } else {
        quicksort(arr, i, right);
    }

    if (created1)
        pthread_join(t1, NULL);
    if (created2)
        pthread_join(t2, NULL);

    sem_post(&sem);
    pthread_exit(NULL);
}

void print_str(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

void print_int(int n) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d", n);
    write(STDOUT_FILENO, buf, len);
}

void print_double(double d) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%.2f", d);
    write(STDOUT_FILENO, buf, len);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_str("Использование: ./p_qsort <размер массива> <число потоков>\n");
        return 1;
    }

    int n = atoi(argv[1]);
    int max_threads = atoi(argv[2]);

    if (n <= 0 || max_threads <= 0) {
        print_str("Ошибка: некорректные аргументы.\n");
        return 1;
    }

    int *arr = malloc(n * sizeof(int));
    if (!arr) {
        print_str("Ошибка: не удалось выделить память.\n");
        return 2;
    }

    srand(42);
    print_str("Исходный массив:\n");
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000000;
        print_int(arr[i]);
        write(STDOUT_FILENO, " ", 1);
    }
    print_str("\n");

    sem_init(&sem, 0, max_threads);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t root;
    sem_wait(&sem);

    Task *rootTask = malloc(sizeof(Task));
    rootTask->arr = arr;
    rootTask->left = 0;
    rootTask->right = n - 1;

    pthread_create(&root, NULL, parallel_qsort, rootTask);
    pthread_join(root, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    print_str("Отсортированный массив:\n");
    for (int i = 0; i < n; i++) {
        print_int(arr[i]);
        write(STDOUT_FILENO, " ", 1);
    }
    print_str("\n");

    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;

    print_str("Время (параллельно, ");
    print_int(max_threads);
    print_str(" потоков): ");
    print_double(elapsed_ms);
    print_str(" мс\n");

    sem_destroy(&sem);
    free(arr);

    return 0;
}
