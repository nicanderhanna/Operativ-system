#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
          
typedef struct page { 
     int page_id;
     int reference_bit;
     struct page *next;
} Node;

Node *top_active = NULL;
Node *top_inactive = NULL;

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

int reference_string[1000]; 
int reference_size = 1000; 
int n; 
int m; 

int *page_stats;
bool *page_seen;
int player_done = 0;

Node* find_page(Node *list_top, int page_id){
     Node *currentTop = list_top; 
     while (currentTop != NULL)
     {
          if (currentTop->page_id == page_id)
          {
               return currentTop; 
          }
          currentTop = currentTop->next; 
     }
     return NULL; 
}

void remove_page(Node **list_top, int page_id){
     Node *current_top = *list_top; 
     Node *previous_node = NULL;

     while (current_top != NULL)
     {
          if (current_top->page_id == page_id)
          {
               if (previous_node == NULL)
               {
                    *list_top = current_top->next; 
               }
               else
               {
                    previous_node->next = current_top->next;
               }
               current_top->next = NULL;
               return;
          }
          
          previous_node = current_top; 
          current_top = current_top->next;     
     }
}

void add_to_end(Node **list_top, Node *page){
     if (*list_top == NULL)
     {
          *list_top = page;
          return;
     }
     
     Node *current_top = *list_top;
     while (current_top->next != NULL)
     {
          current_top = current_top->next;
     }
     current_top->next = page;
}

int list_size(Node *list_top){
     Node *current_top = list_top;
     int number_of_pages = 0;
     while (current_top != NULL)
     {
          number_of_pages++;
          current_top = current_top->next; 
     }
     return number_of_pages;
}

void move_to_inactive(int num){
     for (int i = 0; i < num; i++)
     {
          if (top_active == NULL) return;

          Node *page = top_active;
          top_active = top_active->next;
          page->next = NULL;
          add_to_end(&top_inactive, page);
     }
}

void *player_thread_func() { 
     for (int i = 0; i < reference_size; i++)
     {
          int current_page_id = reference_string[i]; 

          pthread_mutex_lock(&list_mutex);

          Node *page = find_page(top_active, current_page_id);
          bool in_active = (page != NULL);
          
          if (page == NULL)
          {
               page = find_page(top_inactive, current_page_id);
          }
          
          if (page == NULL)
          {
               // New page - create and add to active list
               page = malloc(sizeof(Node));
               page->page_id = current_page_id; 
               page->reference_bit = 1; 
               page->next = NULL;
               page_seen[current_page_id] = true;
               add_to_end(&top_active, page);
          }
          else
          {
               // Page exists - set reference bit and move to end of active list
               page->reference_bit = 1;
               
               if (in_active)
               {
                    remove_page(&top_active, current_page_id);
               }
               else
               {
                    remove_page(&top_inactive, current_page_id);
               }
               add_to_end(&top_active, page);
          }

          // Check if active list is too large
          int size = list_size(top_active);
          if (size > (int)(0.7 * n))
          {
               int move_count = (int)(0.2 * n);
               if (move_count > size)
               {
                    move_count = size;
               }
               move_to_inactive(move_count);
          }
          
          pthread_mutex_unlock(&list_mutex);

          usleep(10);
     }
     
     pthread_mutex_lock(&list_mutex);
     player_done = 1;
     pthread_mutex_unlock(&list_mutex);
     
     pthread_exit(0);
     return NULL; 
}

void *checker_thread_func() { 
     while (1)
     {
          usleep(m);

          pthread_mutex_lock(&list_mutex);

          Node *current_node = top_active;
          while (current_node != NULL)
          {
               if (current_node->reference_bit == 1)
               {
                    page_stats[current_node->page_id]++;
               }
               current_node->reference_bit = 0; 
               current_node = current_node->next;
          }
          
          int done = player_done; 
          pthread_mutex_unlock(&list_mutex);
          
          if (done) break;
     }
     
     pthread_exit(0);
     return NULL;
}

int main(int argc, char *argv[])
{
     if (argc != 3)
     {
          printf("Usage: %s <n> <m>\n", argv[0]);
          return 1;
     }
     
     n = atoi(argv[1]);
     m = atoi(argv[2]);

     page_stats = calloc(n + 1, sizeof(int));
     page_seen = calloc(n + 1, sizeof(bool));

     // Create a random reference string
     srand(time(NULL)); 
     for (int i = 0; i < 1000; i++)
     {
          reference_string[i] = rand() % n; 
     }
    
     pthread_t player;   
     pthread_t checker;    

     pthread_create(&player, NULL, player_thread_func, NULL); 
     pthread_create(&checker, NULL, checker_thread_func, NULL); 
     
     pthread_join(player, NULL);
     pthread_join(checker, NULL);

     printf("Page_Id, Total_Referenced\n");
     for (int i = 0; i < n; i++)
     {
          if (page_seen[i])
          {
               printf("%d, %d\n", i, page_stats[i]);
          }
     }
     
     printf("Pages in active list: ");
     Node *current_node = top_active;
     while (current_node != NULL)
     {
          printf("%d ", current_node->page_id);
          current_node = current_node->next;
     }
     printf("\n");

     printf("Pages in inactive list: ");
     current_node = top_inactive;
     while (current_node != NULL)
     {
          printf("%d ", current_node->page_id);
          current_node = current_node->next;
     }
     printf("\n");
     
     // Free memory
     while (top_active != NULL)
     {
          Node *temp = top_active;
          top_active = top_active->next;
          free(temp);
     }
     while (top_inactive != NULL)
     {
          Node *temp = top_inactive;
          top_inactive = top_inactive->next;
          free(temp);
     }
     free(page_stats);
     free(page_seen);
     pthread_mutex_destroy(&list_mutex);
     
     return 0;
}