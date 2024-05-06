#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int *arr;
    int left;
    int right;
    int depth; // 用于控制创建的线程数
} SortArgs;

void merge(int arr[], int l, int m, int r) {
    int i, j, k, n1 = m - l + 1, n2 = r - m;
    int L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = j = 0; k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        merge(arr, l, m, r);
    }
}

void *mergeSortThread(void *args) {
    SortArgs *arg = (SortArgs *)args;
    if (arg->depth > 0) {
        int m = arg->left + (arg->right - arg->left) / 2;
        pthread_t tid1, tid2;
        SortArgs arg1 = {arg->arr, arg->left, m, arg->depth - 1};
        SortArgs arg2 = {arg->arr, m + 1, arg->right, arg->depth - 1};

        pthread_create(&tid1, NULL, mergeSortThread, &arg1);
        pthread_create(&tid2, NULL, mergeSortThread, &arg2);

        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);

        merge(arg->arr, arg->left, m, arg->right);
    } else {
        mergeSort(arg->arr, arg->left, arg->right);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);

    FILE *file = fopen("input.txt", "r");
    if (!file) {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    int n;
    fscanf(file, "%d", &n);

    int *arr = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        fscanf(file, "%d", &arr[i]);
    }

    SortArgs args = {arr, 0, n - 1, num_threads};
    mergeSortThread(&args);

    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    free(arr);
    fclose(file);
    return 0;
}
