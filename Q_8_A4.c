#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define FILE_SIZE 1048576   // 1MB

int main() {
    int fd;
    char *map;
    pid_t pid;

    fd = open("file_to_map.txt", O_RDWR);

    map = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    pid = fork();

    if (pid == 0) {
        // child process
        printf("Child process (pid=%d); mmap address: %p\n", getpid(), map);

        memcpy(map, "01234", 5);
        msync(map, 5, MS_SYNC); 

        sleep(1);

        printf("Child process (pid=%d); read from mmaped_ptr[4096]: %.5s\n", getpid(), map + 4096);
    }
    else {
        // parent process
        printf("Parent process (pid=%d); mmap address: %p\n", getpid(), map);

        memcpy(map + 4096, "56789", 5);
        msync(map + 4096, 5, MS_SYNC);   

        wait(NULL);

        printf("Parent process (pid=%d); read from mmaped_ptr[0]: %.5s\n", getpid(), map);
    }

    munmap(map, FILE_SIZE); //releases the memory mapped 
    close(fd);
    return 0;
}
