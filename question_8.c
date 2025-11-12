#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>

int main() {
    pid_t pid1, pid2; 
    int N; 
    int firstSum = 0; 
    int secondSum = 0; 

    printf("type a number: ");
    scanf("%d", &N);
    
    int array[N]; 
    srand(time(NULL)); // initialize random number generator

    //Start timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    //The parent will generate a random array with N elements
    for (int i = 0; i < N; i++) {
        array[i] = rand() % 2; 
    }

    //Create pipe 
    int fd1[2], fd2[2]; 
    pipe(fd1);
    pipe(fd2);

    pid1 = fork();

    if (pid1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        // Child 1 sums first half
        close(fd1[0]); //we close the read end since we only want to write
        for (int i = 0; i < N/2; i++) {
            firstSum += array[i];
        }
        write(fd1[1], &firstSum, sizeof(firstSum));
        exit(0);
    }

    pid2 = fork();
    if (pid2 == 0) {
        close(fd2[0]);
        // Child 2 sums second half
        for (int i = N/2; i < N; i++) {
            secondSum += array[i];
        }

        write(fd2[1], &secondSum, sizeof(secondSum));
        exit(0);
    }

    //we can close since we only want to read from them
    close(fd1[1]);
    close(fd2[1]);

    int sum1, sum2;
    read(fd1[0], &sum1, sizeof(sum1));
    read(fd2[0], &sum2, sizeof(sum2));
    //Now we can close these
    close(fd1[0]);
    close(fd2[0]);

    // Parent waits for both
    wait(NULL);
    wait(NULL);

    int total = sum1 + sum2;

    //end time 
    gettimeofday(&end, NULL);

    // Calculate elapsed time in milliseconds
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed_ms += (end.tv_usec - start.tv_usec) / 1000.0;

    printf("Parent (pid=%d): total sum = %d\n", getpid(), total);
    printf("time: %.3f ms\n", elapsed_ms);
    return 0;
}
