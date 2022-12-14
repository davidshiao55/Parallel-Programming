#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MAX 10000

void oddEvenSort(int *arr, int n);

int main(int argc, char *argv[])
{
    int id, numprocs;
    int n;
    int *local, *arr;
    double startwtime = 0.0, endwtime = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    srand(time(NULL) + id);

    if (id == 0)
    {
        scanf("%d", &n);
        arr = (int *)malloc(sizeof(int) * n);
        startwtime = MPI_Wtime();
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //計算每個process處理的數量和gatherv需要的參數
    int num_per_process = n / numprocs;
    int rem = n % numprocs;
    int sendcounts[numprocs], displs[numprocs];
    for (int i = 0; i < numprocs; i++)
    {
        sendcounts[i] = num_per_process;
        displs[i] = i * num_per_process;
    }
    sendcounts[numprocs - 1] += rem;

    // size = maximum possible sum of two local array size
    local = (int *)malloc(sizeof(int) * (2 * num_per_process + rem));
    for (int i = 0; i < sendcounts[id]; i++)
    {
        local[i] = rand() % MAX;
    }

    // sort local array
    oddEvenSort(local, sendcounts[id]);

    // compute partner for odd and even phase
    int oddrank, evenrank;
    if (id % 2 == 0)
    {
        oddrank = id - 1;
        evenrank = id + 1;
    }
    else
    {
        oddrank = id + 1;
        evenrank = id - 1;
    }
    if (oddrank == -1 || oddrank == numprocs)
        oddrank = MPI_PROC_NULL;
    if (evenrank == -1 || evenrank == numprocs)
        evenrank = MPI_PROC_NULL;

    for (int phase = 0; phase < numprocs; phase++)
    {
        // even phase
        if (phase % 2 == 0)
        {
            if (evenrank == MPI_PROC_NULL)
                continue;
            if (id % 2 == 0)
            {
                MPI_Recv(&local[sendcounts[id]], sendcounts[evenrank], MPI_INT, evenrank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                oddEvenSort(local, sendcounts[id] + sendcounts[evenrank]);
                MPI_Send(&local[sendcounts[id]], sendcounts[evenrank], MPI_INT, evenrank, 0, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(local, sendcounts[id], MPI_INT, evenrank, 0, MPI_COMM_WORLD);
                MPI_Recv(local, sendcounts[id], MPI_INT, evenrank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        // odd phase
        else
        {
            if (oddrank == MPI_PROC_NULL)
                continue;
            if (id % 2 == 1)
            {
                MPI_Recv(&local[sendcounts[id]], sendcounts[oddrank], MPI_INT, oddrank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                oddEvenSort(local, sendcounts[id] + sendcounts[oddrank]);
                MPI_Send(&local[sendcounts[id]], sendcounts[oddrank], MPI_INT, oddrank, 0, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(local, sendcounts[id], MPI_INT, oddrank, 0, MPI_COMM_WORLD);
                MPI_Recv(local, sendcounts[id], MPI_INT, oddrank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }

    MPI_Gatherv(local, sendcounts[id], MPI_INT, arr, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    endwtime = MPI_Wtime();

    if (id == 0)
    {
        printf("sorted array : \n");
        for (int i = 0; i < n; i++)
        {
            printf("%d ", arr[i]);
        }
        printf("\n");
        printf("process finish in %lf time\n", endwtime - startwtime);
    }

    MPI_Finalize();

    return 0;
}

void oddEvenSort(int *arr, int n)
{
    int phase, i, tmp;

    for (phase = 0; phase < n; phase++)
    {
        if (phase % 2 == 0)
        {
            for (i = 1; i < n; i += 2)
            {
                if (arr[i - 1] > arr[i])
                {
                    tmp = arr[i - 1];
                    arr[i - 1] = arr[i];
                    arr[i] = tmp;
                }
            }
        }
        else
        {
            for (i = 1; i < n - 1; i += 2)
            {
                if (arr[i] > arr[i + 1])
                {
                    tmp = arr[i + 1];
                    arr[i + 1] = arr[i];
                    arr[i] = tmp;
                }
            }
        }
    }
}