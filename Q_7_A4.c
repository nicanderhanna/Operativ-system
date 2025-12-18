#define _POSIX_C_SOURCE 199309L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BLOCK_SIZE 16384


/* structure to pass data to threads */
struct thread_data {
    int fd;
    char *buffer;
    int start;
    int end;
};

/* writer thread */
void *writer(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;

    for (int i = data->start; i < data->end; i++) {
        off_t offset = i * BLOCK_SIZE;
        pwrite(data->fd,
               data->buffer + offset,
               BLOCK_SIZE,
               offset);
    }

    return NULL;
}

/* reader thread */
void *reader(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    char temp[BLOCK_SIZE];

    for (int i = data->start; i < data->end; i++) {
        off_t offset = i * BLOCK_SIZE;
        pread(data->fd,
              temp,
              BLOCK_SIZE,
              offset);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int N = atoi(argv[1]);   // number of bytes
    int P = atoi(argv[2]);   // number of threads

    /* allocate buffer */
    char *buffer = malloc(N);
    for (int i = 0; i < N; i++) {
        buffer[i] = 'A';
    }

    pthread_t threads[P];
    struct thread_data tdata[P];

   int REQUESTS = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;


    int requests_per_thread = REQUESTS / P;

    /* ---------------- WRITE ---------------- */
    int fd = open("testfile.dat", O_CREAT | O_RDWR | O_TRUNC, 0644);

    struct timespec start_w, end_w;
    clock_gettime(CLOCK_MONOTONIC, &start_w);

    for (int i = 0; i < P; i++) {
        tdata[i].fd = fd;
        tdata[i].buffer = buffer;
        tdata[i].start = i * requests_per_thread;
        tdata[i].end = (i == P - 1) ? REQUESTS
                                    : (i + 1) * requests_per_thread;

        pthread_create(&threads[i], NULL, writer, &tdata[i]);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_w);
    close(fd);

    double write_time =
        (end_w.tv_sec - start_w.tv_sec) +
        (end_w.tv_nsec - start_w.tv_nsec) / 1e9;

    double write_mb = (double)(REQUESTS * BLOCK_SIZE) / (1024 * 1024);

    printf("Write %.2f MB with %d threads in %.3f seconds (%.2f MB/s)\n",
           write_mb, P, write_time, write_mb / write_time);

    /* ---------------- READ ---------------- */
    fd = open("testfile.dat", O_RDONLY);

    struct timespec start_r, end_r;
    clock_gettime(CLOCK_MONOTONIC, &start_r);

    for (int i = 0; i < P; i++) {
        pthread_create(&threads[i], NULL, reader, &tdata[i]);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_r);
    close(fd);

    double read_time =
        (end_r.tv_sec - start_r.tv_sec) +
        (end_r.tv_nsec - start_r.tv_nsec) / 1e9;

    printf("Read  %.2f MB with %d threads in %.3f seconds (%.2f MB/s)\n",
           write_mb, P, read_time, write_mb / read_time);

    free(buffer);
    return 0;
}
