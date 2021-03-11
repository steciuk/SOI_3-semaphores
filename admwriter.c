/** Compilation: gcc -o memwriter memwriter.c -lrt -lpthread **/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>       
#include <fcntl.h>          
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "shmem.h"
#include <pthread.h>

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}

typedef struct node {
    char* val;
    struct node * next;
} node_t;

node_t* init(char* msg){
    node_t * head = NULL;
    head = malloc(sizeof(node_t));

    head->val = msg;
    head->next = NULL;
    return head;
}

void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
        printf("%s\n", current->val);
        current = current->next;
    }
}

void push_end(node_t * head, char* val) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = malloc(sizeof(node_t));
    current->next->val = val;
    current->next->next = NULL;
}

void push_begin(node_t ** head, char* val) {
    node_t * new_node;
    new_node = malloc(sizeof(node_t));

    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

char* pop(node_t** head) {
    char* retval;
    node_t * next_node = NULL;

    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;

    return retval;
}

char* readHead(node_t** head) {
    char* retval;
    node_t * next_node = NULL;

    retval = (*head)->val;

    return retval;
}

char* remove_last(node_t * head) {
    char* retval;

    if (head->next == NULL) {
        retval = head->val;
        free(head);
        return retval;
    }

    node_t * current = head;
    while (current->next->next != NULL) {
        current = current->next;
    }

    retval = current->next->val;
    free(current->next);
    current->next = NULL;
    return retval;

}

sem_t tr_end;
sem_t list_full;
sem_t list_empty;
sem_t list_lock;
node_t* messages;

void *listener(void *vargp) 
{ 
    sleep(1);
    int fd = shm_open(BackingFile,     
                O_RDWR | O_CREAT, 
                AccessPerms);    
    if (fd < 0) report_and_exit("Can't open shared mem segment...");

    ftruncate(fd, ByteSize); 

    caddr_t memptr = mmap(NULL,     
                ByteSize,  
                PROT_READ | PROT_WRITE,
                MAP_SHARED, 
                fd,         
                0);         
    if ((caddr_t) -1  == memptr) report_and_exit("Can't get segment...");

    fprintf(stderr, "shared mem address: %p [0..%d]\n", memptr, ByteSize - 1);
    fprintf(stderr, "backing file:       /dev/shm%s\n", BackingFile );


    sem_t* semptr = sem_open(SemaphoreName,
    O_CREAT,       
    AccessPerms,   
    1);            

    sem_t* semptr_empty = sem_open(SemaphoreName2, 
    O_CREAT,      
    AccessPerms,   
    0);            

    sem_t* semptr_full = sem_open(SemaphoreName3, 
    O_CREAT,      
    AccessPerms,   
    1);            
 



    while(sem_trywait(&tr_end) != 0)
    {
        //sleep(3); 
        sem_wait(semptr_empty);

            sem_wait(semptr);
            int i;
            
            char * str2 = malloc(sizeof(char)*MAX_LIMIT);
            
            for (i = 0; i < MAX_LIMIT; i++)
            {
                str2[i] = *(memptr + i);
                /*write(STDOUT_FILENO, memptr + i, 1); /* one byte at a time */
            }
            sem_post(semptr);

            if(str2[0] == 'u')
            {
                sem_wait(&list_full);
                sem_wait(&list_lock);

                if(messages == NULL) messages = init(str2);
                else push_end(messages, str2);

                sem_post(&list_lock);
                sem_post(&list_empty);
            }
            else if (str2[0] == 'v')
            { 
                sem_wait(&list_full);
                sem_wait(&list_lock);
                
                if(messages == NULL) messages = init(str2);
                else push_begin(&messages, str2);

                sem_post(&list_lock);
                sem_post(&list_empty);
            }   

        sem_post(semptr_full);
    }

  
    munmap(memptr, ByteSize); 
    close(fd);
    sem_close(semptr);
    sem_close(semptr_empty);
    shm_unlink(BackingFile); 
    printf("THREAD ENDED!");
    return NULL; 
}

int main() {

    sem_init(&list_full, 0, MAX_MSG);
    sem_init(&list_empty, 0, 0);
    sem_init(&list_lock, 0, 1);
    sem_init(&tr_end, 0, 0);

    pthread_t thread_id; 
    pthread_create(&thread_id, NULL, listener, NULL); 

    sleep(2);
  
    int opt = 0;

  do
  {
    printf("1 - send\n");
    printf("2 - read\n");
    printf("3 - clear\n");
    printf("0 - quit\n");
    printf("Enter number: ");
    scanf("%d", &opt);

    if(opt == 1)
    {
        if(sem_trywait(&list_empty) == 0)
        {
            sem_wait(&list_lock);
            
            printf("%s\n", pop(&messages));
            
            sem_post(&list_lock);
            sem_post(&list_full);
        }
        else
        {
            printf("No mesagges!\n");
        }  
    }
    else if(opt == 2)
    {
        if(sem_trywait(&list_empty) == 0)
        {
            sem_post(&list_empty);
            sem_wait(&list_lock);

            print_list(messages);
            
            sem_post(&list_lock);
        }
        else
        {
            printf("No mesagges!\n");
        }  
    }
    else if(opt == 3)
    {
        
        while(sem_trywait(&list_empty) == 0)
        {
            sem_wait(&list_lock);
            
            pop(&messages);
            
            sem_post(&list_lock);
            sem_post(&list_full);
        }

        printf("All mesagges deleted!\n");

           
        // char * str = malloc(sizeof(char)*MAX_LIMIT);
        // printf("Enter message: ");
        // getchar();
        // fgets(str, MAX_LIMIT, stdin); 
             
        // if(messages == NULL) messages = init(str);
        // else push_end(messages, str);

        // print_list(messages);
    }
        
    

  } while (opt != 0);


while(sem_trywait(&list_empty) == 0)
{
    sem_wait(&list_lock);
            
    pop(&messages);
            
    sem_post(&list_lock);
    sem_post(&list_full);
}
  

    sem_post(&tr_end);
    pthread_join(thread_id, NULL);
    return 0;
}


