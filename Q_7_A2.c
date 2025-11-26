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
struct timeval start, end;
double *array;
int array_size;
int bin = 30;
int *partial_hist;
pthread_mutex_t hist_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_id = 0; // global för att ge unika IDs till trådarna

void *thread_func(); /* the thread function */



int main(int argc, char *argv[])
{
num_threads = atoi(argv[1]);
array_size = atoi(argv[2]);
array = malloc(array_size * sizeof(double));
/* Initialize an array of random values */
srand(time(NULL));
for (int i=0; i<array_size; i++){
    array[i] = (double)rand() / (double)RAND_MAX;
}

 // ----------------------------- Seriell histogram ------------------------
int hist_serial[bin];
for (int i = 0; i < bin; i++) hist_serial[i] = 0;

gettimeofday(&start, NULL);
for (int i = 0; i < array_size; i++) {
    int bin_index = (int)(array[i] * bin);
    if (bin_index == bin) bin_index = bin - 1;
    hist_serial[bin_index]++;
}
gettimeofday(&end, NULL);

double time_hist_serial = (end.tv_sec - start.tv_sec) * 1000.0;
time_hist_serial += (end.tv_usec - start.tv_usec) / 1000.0;

printf("Serial Histogram:\n");
for (int i = 0; i < bin; i++)
    printf("Bin %d: %d\n", i, hist_serial[i]);
printf("Serial histogram time = %.3f ms\n", time_hist_serial);

/*-------------------------------Serial histogram ----------------------*/

/*--------------------------Parallel histogram---------------------------- */
// Allokera partial_hist INNAN vi startar threads
partial_hist = calloc(bin, sizeof(int));
pthread_t *workers = malloc(num_threads * sizeof(pthread_t));

double time_hist_parallel = 0.0;

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

time_hist_parallel = (end.tv_sec - start.tv_sec) * 1000.0;
time_hist_parallel += (end.tv_usec - start.tv_usec) / 1000.0;

printf("\nParallel Histogram:\n");
for (int i = 0; i < bin; i++)
    printf("Bin %d: %d\n", i, partial_hist[i]);
printf("Parallel histogram time = %.3f ms\n", time_hist_parallel);


/*--------------------------Parallel histogram---------------------------- */


free(array);
free(partial_hist);
free(workers);

return 0;
}



/* --------------------------thread function----------------------- */

void *thread_func() {
/* Assign each thread an id so that they are unique in range [0, num_thread -1
] */
pthread_mutex_lock(&id_mutex);
int my_id = next_id++;
pthread_mutex_unlock(&id_mutex);

/* Perform Partial Parallel Sum Here */
int chunk = array_size / num_threads;
int start_index = my_id * chunk;
int end_index = (my_id == num_threads-1) ? array_size : start_index + chunk;

int *thread_hist = calloc(bin, sizeof(int));

for (int i = start_index; i < end_index; i++) {
    int bin_index = (int)(array[i] * bin);
    if (bin_index == bin) bin_index = bin-1;
    thread_hist[bin_index]++;
}

pthread_mutex_lock(&hist_mutex);
for (int i = 0; i < bin ; i++)
    partial_hist[i] += thread_hist[i];
pthread_mutex_unlock(&hist_mutex);

free(thread_hist);
pthread_exit(0);
}