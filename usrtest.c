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

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); 
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main() {
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

  
  int opt = 0;
  int tim = 0;
  
  char * str = malloc(sizeof(char)*MAX_LIMIT);
  
  do
  {
    printf("usrID: ");
    scanf("%s", str);
    printf("Num of mesgs: ");
    scanf("%d", &opt);
    printf("Interwal: ");
    scanf("%d", &tim);

    for(int i=0; i<opt; i++)
    {
      sleep(tim);
      char str3[12];
      sprintf(str3, "%d", i);
      if(sem_wait(semptr_full) == 0)
      {
        if (sem_trywait(semptr) == 0)
        {


            char* s = concat("u", str);   
            char* s2 = concat(s, ": mes"); 



            char* s3 = concat(s2, str3);
            strcpy(memptr, s3);
            printf("mes%d sent\n", i);
            
            sem_post(semptr_empty);
            free(s); 
            free(s2);
            free(s3);
            
            if (sem_post(semptr) < 0) report_and_exit("sem_post");
        }
        else
        {
        printf("Sem locked!\n");
        }
      }
      else
      {
      printf("Buffer full\n");
      }  
    }


  } while (opt != 0);
  

  


 
  
 
  munmap(memptr, ByteSize); 
  close(fd);
  sem_close(semptr);
  shm_unlink(BackingFile); 
  return 0;
}


