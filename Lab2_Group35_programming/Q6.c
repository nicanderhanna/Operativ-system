#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>

int num_threads = 0;
float array[1000000];
float *partial_sums;
struct timeval start, end;

pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_id = 0;

void *thread_func(); /* the thread function */

int main(int argc, char *argv[])
{
num_threads = atoi(argv[1]);
/* Initialize an array of random values */
srand(time(NULL));
for (int i=0; i<1000000; i++){
  float x = rand() % 2;
  array[i] = x;
}
/* Perform Serial Sum */
float sum_serial = 0.0;
double time_serial = 0.0;
//Timer Begin

gettimeofday(&start, NULL);

for (int i=0; i<1000000 ; i++){
  int x = array[i];
  sum_serial += x;
}

gettimeofday(&end, NULL);
//Timer End

time_serial = (end.tv_sec - start.tv_sec) * 1000.0;
time_serial += (end.tv_usec - start.tv_usec) / 1000.0;

printf("Serial Sum = %f, time = %.3f \n", sum_serial, time_serial);
/* Create a pool of num_threads workers and keep them in workers */
partial_sums = malloc(num_threads * sizeof(float));
pthread_t *workers = malloc(num_threads * sizeof(pthread_t));
double time_parallel = 0.0;
double sum_parallel = 0.0;

//Timer Begin
gettimeofday(&start, NULL);
for (int i = 0; i < num_threads; i++) {
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_create(&workers[i], &attr, thread_func, NULL);
}
for (int i = 0; i < num_threads; i++)
pthread_join(workers[i], NULL);
//Timer End
gettimeofday(&end, NULL);

for (int i = 0; i < num_threads; i++)
        sum_parallel += partial_sums[i];

time_parallel = (end.tv_sec - start.tv_sec) * 1000.0;
time_parallel += (end.tv_usec - start.tv_usec) / 1000.0;

printf("Parallel Sum %f = , time = %.3f \n", sum_parallel, time_parallel);
/*free up resources properly */
free(workers);
free(partial_sums);

return 0;
}


void *thread_func() {
/* Assign each thread an id so that they are unique in range [0, num_thread -1
] */
pthread_mutex_lock(&id_mutex);
int my_id = next_id++;
pthread_mutex_unlock(&id_mutex);

/* Perform Partial Parallel Sum Here */
int chunk = 1000000 / num_threads;
int start_index = my_id * chunk;
int end_index = (my_id == num_threads-1) ? 1000000 : start_index + chunk;

float my_sum = 0.0;

for (int i = start_index; i < end_index; i++) {
    my_sum += array[i];
}

partial_sums[my_id] = my_sum;

printf("Thread %d sum = %f\n", my_id, my_sum);
pthread_exit(0);
}