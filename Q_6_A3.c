/*RUN PROGAM
gcc -o Q_6_A3 Q_6_A3.c -Wall
/usr/bin/time --verbose ./Q_6_A3 128
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    
    int num_pages = atoi(argv[1]);
    
 
    int page_size = getpagesize();
    printf("System page size: %d bytes\n", page_size);
    
    
    size_t total_bytes = (size_t)num_pages * page_size;
    printf("Allocating %d pages (%zu bytes)\n", num_pages, total_bytes);
    
    //Allocate memory
    void *ptr = malloc(total_bytes);
    if (ptr == NULL) {
        fprintf(stderr, "Error: malloc failed\n");
        return 1;
    }
    
    printf("Memory allocated successfully at address: %p\n", ptr);
    
    memset(ptr, 0, total_bytes);
    
    //keeps the program running
    printf("Press Enter to free memory and exit...\n");
    getchar();
    
   //free 
    free(ptr);
    printf("Memory freed\n");
    
    return 0;
}