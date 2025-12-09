#define _GNU_SOURCE


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h> 
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>



// Funktion för att beräkna tidsskillnaden i nanosekunder (ns)
long diff_time_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
}


int main(int argc, char** argv){

  // time_t start,end;
  struct timespec start, end;
    
  int num_pages = atoi(argv[1]);
  int page_size = getpagesize();
  long total_size = (long)num_pages * page_size;
  
  printf("Allocating %d pages of %d bytes \n", num_pages, page_size);

  char *addr;
  
  // @Add the start of Timer here
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
      perror("clock_gettime start"); exit(1);
  }

  // Option 1: @Add mmap below to allocate num_pages anonymous pages 
  // -------------------------------Option 1---------------------
   addr = (char*) mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
 // -------------------------------Option 1---------------------

  // -------------------------------Option 2---------------------

  // Option 2: @Add mmap below to allocate num_pages anonymous pages but using Ausing "huge" pages
  //addr = (char*) mmap( ... );
  //addr = (char*) mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
  
   // -------------------------------Option 1---------------------

  if (addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  //the code below updates the pages
  char c = 'a';
  for(int i=0; i<num_pages; i++){
    addr[i*page_size] = c;
    c ++;
  }

  // @Add the end of Timer here
 if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
      perror("clock_gettime end"); exit(1);
  }
  
  // @Add printout of elapsed time in cycles
  long time_it_took_ns = diff_time_ns(start, end);
  double time_it_took_s = (double)time_it_took_ns / 1000000000.0;
  printf("Elapsed time (in seconds): %lf\n", time_it_took_s);  
  
  for(int i=0; (i<num_pages && i<16); i++){
    printf("%c ", addr[i*page_size]);
  }
  printf("\n");
  
  munmap(addr, page_size*num_pages);
  
  return 0;
}
