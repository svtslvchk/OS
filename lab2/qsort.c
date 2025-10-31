#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap(int *a, int *b) {
    int t = *a; 
    *a = *b; 
    *b = t;
}

void quicksort(int *arr, int left, int right) {
    if (left >= right) return;
    int pivot = arr[(left + right) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (arr[i] < pivot) {
            i++;
        }

        while (arr[j] > pivot) {
            j--;
        }

        if (i <= j) {
            swap(&arr[i++], &arr[j--]);
        }
    }

    if (left < j) {
        quicksort(arr, left, j);
    }

    if (i < right) {
        quicksort(arr, i, right);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s <размер массива>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int *arr = malloc(n * sizeof(int));
    if (!arr) {
        perror("Ошибка выделения памяти");
        return 2;
    }

    printf("Изначальный массив\n");
    srand(42);
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000000;
        printf("%d ", arr[i]);
    }

    printf("\n");

    clock_t start = clock();
    quicksort(arr, 0, n - 1);
    clock_t end = clock();

    printf("Массив после сортировки\n");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }

    printf("\n");

    double time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    printf("Время (последовательно): %.2f мс\n", time_ms);

    free(arr);
    return 0;
}
