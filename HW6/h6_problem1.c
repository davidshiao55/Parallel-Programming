#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <omp.h>
// #include <mpi.h>

#define N 48
#define K 10000
#define V 0.7
#define ALPHA 1
#define BETA 1
#define Q 1
#define NTRHEADS 4

typedef struct Edge
{
    int i;
    int j;
} Edge;

typedef struct Stack
{
    Edge edges[N];
    int top;
} Stack;

void push(Stack *s, int i, int j);
Edge pop(Stack *s);
void clear(Stack *s);

int main()
{
    FILE *fp;
    int min_length = INT_MAX;
    int adj_matrix[N][N];
    double pheromone[N][N];
    fp = fopen("att48_d.txt", "r");

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            fscanf(fp, "%d", &adj_matrix[i][j]);
            pheromone[i][j] = 1;
        }
    }

    int count = 0;
    // initialize pheromone matrix
    srand(time(NULL));
    omp_set_num_threads(NTRHEADS);
    while (1)
    {
        Stack stack[K];
        int l[K];

#pragma omp parallel for
        for (int i = 0; i < K; i++)
        {
            int start = rand() % N;
            int city = start;
            bool visited[N] = {0};
            int visit_count = 0;
            l[i] = 0;
            clear(&stack[i]);
            while (1)
            {
                if (visit_count == N - 1 && adj_matrix[city][start])
                {
                    l[i] += adj_matrix[city][start];
                    push(&stack[i], city, start);
                    if (l[i] < min_length)
                    {
                        count = 0;
                        min_length = l[i];
                    }
                    break;
                }

                double p[N] = {0};
                double d = 0;
                for (int j = 0; j < N; j++)
                {
                    if (visited[j] || !adj_matrix[city][j])
                        continue;
                    d += pow(pheromone[city][j], (double)ALPHA) * pow((double)1 / adj_matrix[city][j], (double)BETA);
                }

                for (int j = 0; j < N; j++)
                {
                    if (visited[j] || !adj_matrix[city][j])
                        p[j] = 0;
                    else
                        p[j] = pow(pheromone[city][j], (double)ALPHA) * pow((double)1 / adj_matrix[city][j], (double)BETA) / d;
                }

                double r = (double)rand() / RAND_MAX;
                int next = 0;
                while (r > p[next])
                {
                    r -= p[next++];
                }
                l[i] += adj_matrix[city][next];
                push(&stack[i], city, next);
                visited[city] = true;
                visit_count++;
                city = next;
            }
        }

        for (int j = 0; j < N; j++)
        {
            for (int k = 0; k < N; k++)
                pheromone[j][k] *= V;
        }
        for (int i = 0; i < K; i++)
        {
            while (stack[i].top != -1)
            {
                Edge e = pop(&stack[i]);
                pheromone[e.i][e.j] += (1 - V) * (double)Q / l[i];
            }
        }
        printf("%d : %d\n", count++, min_length);
    }
    return 0;
}

void push(Stack *s, int i, int j)
{
    ++(s->top);
    s->edges[s->top].i = i;
    s->edges[s->top].j = j;
}

Edge pop(Stack *s)
{
    return s->edges[(s->top)--];
}

void clear(Stack *s)
{
    s->top = -1;
}