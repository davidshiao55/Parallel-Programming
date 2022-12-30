#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <stdbool.h>

#define NTHREADS 8
#define PRODUCER 0
#define CONSUMER 1
#define BUFF_SIZE 4096

typedef struct QNode
{
    char *data;
    struct QNode *next;
} QNode;

typedef struct Queue
{
    struct QNode *front, *rear;
} Queue;

QNode *newNode(char *data);
Queue *createQueue();
void detroyQueue(Queue *q);
void enQueue(Queue *q, char *data);
void deQueue(Queue *q);

int main()
{
    int count = 0;
    int counter = 0;
    int read_finish = 0;
    Queue *q = createQueue();
    FILE *fp[NTHREADS / 2];
    const char keyword[] = "my";
    fp[0] = fopen("file1.txt", "r");
    fp[1] = fopen("file2.txt", "r");
    fp[2] = fopen("file3.txt", "r");
    fp[3] = fopen("file4.txt", "r");
    omp_set_num_threads(NTHREADS);
#pragma omp parallel reduction(+ \
                               : count)
    {
        int id = omp_get_thread_num();
        int job = (id < NTHREADS / 2) ? PRODUCER : CONSUMER;
        if (job == PRODUCER)
        {
            while (1)
            {
                char *line = (char *)malloc(sizeof(char) * BUFF_SIZE);
                if (!fgets(line, BUFF_SIZE, fp[id]))
                {
#pragma omp critical
                    read_finish++;
                    break;
                }
#pragma omp critical
                {
                    counter++;
                    enQueue(q, line);
                }
            }
        }
        else if (job == CONSUMER)
        {
            while (counter > 0 || read_finish != NTHREADS / 2)
            {
                bool qempty = false;
                char *line;
#pragma omp critical
                {
                    if (!q->front)
                    {
                        qempty = true;
                    }
                    else
                    {
                        line = q->front->data;
                        counter--;
                        deQueue(q);
                    }
                }
                if (qempty)
                    continue;

                const char d[] = " \n";
                char *token = strtok(line, d);
                while (token != NULL)
                {
                    if (!strcmp(token, keyword))
                        count++;
                    token = strtok(NULL, d);
                }
                free(line);
            }
        }
    }
    printf("keyword \"%s\" : %d\n", keyword, count);
    for (int i = 0; i < NTHREADS / 2; i++)
        fclose(fp[i]);
    return 0;
}

QNode *newNode(char *data)
{
    struct QNode *temp = (struct QNode *)malloc(sizeof(struct QNode));
    temp->data = data;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
Queue *createQueue()
{
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

void detroyQueue(Queue *q)
{
    while (q->front)
    {
        deQueue(q);
    }
    free(q);
}

// The function to add a key k to q
void enQueue(struct Queue *q, char *data)
{
    // Create a new LL node
    struct QNode *temp = newNode(data);

    // If queue is empty, then new node is front and rear
    // both
    if (q->front == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a key from given queue q
void deQueue(struct Queue *q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return;

    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}