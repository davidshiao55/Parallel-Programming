#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>
#include <sys/time.h>

#define N 100000
#define RANGE 200000
#define NTHREADS 4

void Count_sort(int a[], int n);
void Count_sort_p(int a[], int n);

int cmpfunc(const void *a, const void *b);

int main()
{
    int a[N], _a[N], __a[N];
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        a[i] = rand() % RANGE + 1;
    }
    memcpy(_a, a, N * sizeof(int));
    memcpy(__a, a, N * sizeof(int));

    long start, end;
    long qsort_time, csort_time, csort_time_p;
    struct timeval timecheck;

    gettimeofday(&timecheck, NULL);
    start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

    Count_sort(a, N);

    gettimeofday(&timecheck, NULL);
    end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
    csort_time = end - start;

    gettimeofday(&timecheck, NULL);
    start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

    Count_sort_p(_a, N);

    gettimeofday(&timecheck, NULL);
    end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
    csort_time_p = end - start;

    gettimeofday(&timecheck, NULL);
    start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

    qsort(__a, N, sizeof(int), cmpfunc);

    gettimeofday(&timecheck, NULL);
    end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
    qsort_time = end - start;

    printf("serial csort took %ld milliseconds elapsed\n", csort_time);
    printf("parallel csort took %ld milliseconds elapsed\n", csort_time_p);
    printf("serial qsort took %ld milliseconds elapsed\n", qsort_time);
    return 0;
}

void Count_sort_p(int a[], int n)
{
    int i, j, count;
    int *temp = malloc(n * sizeof(int));
#pragma omp parallel for shared(a, temp) private(i, j, count)
    for (i = 0; i < n; i++)
    {
        count = 0;
        for (j = 0; j < n; j++)
            if (a[j] < a[i])
                count++;
            else if (a[j] == a[i] && j < i)
                count++;
#pragma omp critical
        temp[count] = a[i];
    }
    memcpy(a, temp, n * sizeof(int));
    free(temp);
}
/* Count sort */

int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

void Count_sort(int a[], int n)
{
    int i, j, count;
    int *temp = malloc(n * sizeof(int));
    for (i = 0; i < n; i++)
    {
        count = 0;
        for (j = 0; j < n; j++)
            if (a[j] < a[i])
                count++;
            else if (a[j] == a[i] && j < i)
                count++;
        temp[count] = a[i];
    }
    memcpy(a, temp, n * sizeof(int));
    free(temp);
}
/* Count sort */