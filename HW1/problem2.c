#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

double randfrom(double min, double max);

int main(int argc, char *argv[])
{
    int id = 0, numprocs; /* process id */
    double startTime = 0.0, totalTime = 0.0;
    int partner;
    long long int toss, number_of_tosses, number_in_circle = 0;
    double x, y, distance_squared, pi_estimate, recv_pi;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (id == 0)
    {
        scanf("%lld", &number_of_tosses);
    }
    MPI_Bcast(&number_of_tosses, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    srand(time(NULL));

    if (id == 0)
    {
        startTime = MPI_Wtime();
    }

    for (toss = 0; toss < number_of_tosses / numprocs; toss++)
    {
        x = randfrom(-1, 1); // random double between - 1 and 1;
        y = randfrom(-1, 1); // random double between - 1 and 1;
        distance_squared = x * x + y * y;
        if (distance_squared <= 1)
            number_in_circle++;
    }
    pi_estimate = 4 * (double)number_in_circle / ((double)number_of_tosses);

    for (int s = 2; s <= numprocs; s *= 2)
    {
        if (id % s == s / 2)
        {
            partner = id - s / 2;
            MPI_Send(&pi_estimate, 1, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD);
        }
        else if (id % s == 0)
        {
            partner = id + s / 2;
            MPI_Recv(&recv_pi, 1, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            pi_estimate += recv_pi;
        }
        else
        {
            break;
        }
    }

    if (id == 0)
    {
        totalTime = MPI_Wtime() - startTime;
        printf("estimated pi = %lf\nprocess finish in %lf second\n", pi_estimate, totalTime);
    }
    MPI_Finalize();

    return 0;
}

double randfrom(double min, double max)
{
    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}
