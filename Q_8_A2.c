//COMPILE CODE WITH: gcc -pthread Q_8_A2.c -o Q_8_A2

#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
     
int num_threads = 0;

pthread_mutex_t stack_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;

int node_counter = 0;
int id_counter = 0; 

typedef struct node { 
     int node_id;      //a unique ID assigned to each node
     struct node *next;
} Node;

Node *top; // top of stack

/*Option 1: Mutex Lock*/
void push_mutex() { 
     Node *old_node;
     Node *new_node;
     new_node = malloc(sizeof(Node)); 

     //Lock the mutex 
     pthread_mutex_lock(&stack_mutex); 
     new_node->node_id = node_counter;
     node_counter++; 
     
     new_node->next = top; //since it is LIFO the next one will be the one that is currently on the top 

     top = new_node; //the last in is now the top

     pthread_mutex_unlock(&stack_mutex);
     //update top of the stack below
     //assign a unique ID to the new node
}

int pop_mutex() { 
     Node *old_node;
     Node *new_node;

     //update top of the stack below
     pthread_mutex_lock(&stack_mutex);
     old_node = top; 
     top = top->next;
     pthread_mutex_unlock(&stack_mutex);


     return old_node->node_id;
}

/*Option 2: Compare-and-Swap (CAS)*/
void push_cas() { 
     Node *old_node;
     Node *new_node;
     new_node = malloc(sizeof(Node)); 

     //update top of the stack below
     //assign a unique ID to the new node
     pthread_mutex_lock(&stack_mutex);
     new_node->node_id = node_counter;
     node_counter++; 
     pthread_mutex_unlock(&stack_mutex);

     do
     {
          old_node = top; 
          new_node->next = old_node;
     } while (!__sync_bool_compare_and_swap(&top, old_node, new_node)); //if top is still old_node, then replace it with new_node

}

int pop_cas() { 
     Node *old_node;      
     Node *new_node;
      //update top of the stack below
     do {
          old_node = top;              // Read current top
          if (old_node == NULL)        // Check for empty stack
               return -1;
          new_node = old_node->next;    // Prepare new top
     } while (!__sync_bool_compare_and_swap(&top, old_node, new_node)); //If top is still old_node, replace it with new_node
     return old_node->node_id;
}

/* the thread function */
void *thread_func(int opt) { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     pthread_mutex_lock(&id_mutex);
     int my_id = id_counter;
     id_counter++; 
     pthread_mutex_unlock(&id_mutex);

     if( opt==0 ){
          push_mutex();push_mutex();pop_mutex();pop_mutex();push_mutex();
     }else{
          push_cas();push_cas();pop_cas();pop_cas();push_cas();
     }
     
     printf("Thread %d: exit\n", my_id);
     pthread_exit(0);
}

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);
     top = NULL; 
     /* Option 1: Mutex */ 
     pthread_t *workers = malloc(num_threads* sizeof(pthread_t));
     for (int i = 0; i < num_threads; i++) { 
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, (void *(*)(void *))thread_func, (void *)0); 
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

     //Print out all remaining nodes in Stack
     printf("Mutex: Remaining nodes \n");
     int nodesLeft = 0;
     Node *currentTop = top; 

     while (currentTop != NULL)
     {
          printf("Node ID: %d\n", currentTop->node_id);
          nodesLeft++; 
          currentTop = currentTop->next; 
     }
     printf("Total Mutex nodes remaining: %d\n", nodesLeft);

     
     /*free up resources properly */
     currentTop = top; 
     Node *temp; 
     while (currentTop != NULL)
     {
          temp = currentTop; 
          currentTop = currentTop->next; 
          free(temp);
     }

     //reset for cas test 
     nodesLeft = 0; 
     node_counter = 0; 
     id_counter = 0;
     top = NULL; 
     
     /* Option 2: CAS */ 
          for (int i = 0; i < num_threads; i++) { 
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr,  (void *(*)(void *))thread_func, (void *)1); 
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

     //Print out all remaining nodes in Stack
     printf("CAS: Remaining nodes \n");
     currentTop = top; 

     while (currentTop != NULL)
     {
          printf("Node ID: %d\n", currentTop->node_id);
          nodesLeft++; 
          currentTop = currentTop->next; 
     }
     printf("Total CAS nodes remaining: %d\n", nodesLeft);
     
     /*free up resources properly */
     currentTop = top; 
     while (currentTop != NULL) {
          temp = currentTop; 
          currentTop = currentTop->next; 
          free(temp);
     }

     free(workers);
     pthread_mutex_destroy(&id_mutex);
     pthread_mutex_destroy(&stack_mutex);

}