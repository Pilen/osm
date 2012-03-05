#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREADS_MAX 5
#define ITERATIONS 5

// error function, checking if val is 0, expecting it to be errno otherwise.
/*
void checkResults(char* msg, int val) {
  if (val == 0) 
    return;
  fprintf(stderr, "%s: %s\n", msg, strerror(val));
  exit(EXIT_FAILURE);
}
*/

int counter = 0;
int readcounter, writecounter = 0;
int reads = 0, writes = 0;
int maxcounter = 12;
pthread_mutex_t counter_mutex;
pthread_mutex_t write_mutex;
pthread_cond_t counter_cond;

void *write_counter(void *p) {
  long id = (long)p;
  int i = ITERATIONS;
  //  while (i --> 0) {
    pthread_mutex_lock(&write_mutex);
      //   printf("writer %ld getting write lock \n", id);
    counter++;
    usleep(100);
    printf("Wrote: thread %ld, counter = %d.\n", id, counter);
    writes++;
    pthread_mutex_unlock(&write_mutex);
    //   printf("writer %ld unlocked write lock \n", id);
    // }
}

void *read_counter(void *p) {
  long id = (long)p;
  int i = ITERATIONS;
  while (i-- > 0) {
    pthread_mutex_lock(&counter_mutex);
    //    printf("reader %ld getting write lock \n", id);
    if (++readcounter == 1) {
      //      printf("reader %ld getting write lock \n", id);
      pthread_mutex_lock(&write_mutex);
      }
    pthread_mutex_unlock(&counter_mutex);
    
    usleep(50);
    printf("Read: thread %ld, counter = %d.\n", id, counter);

    pthread_mutex_lock(&counter_mutex);
    if (--readcounter == 0)
      pthread_mutex_unlock(&write_mutex);
    //    printf("reader %ld unlocked write lock \n", id);
    pthread_mutex_unlock (&counter_mutex);     
    //    printf("reader %ld unlocked writer lock \n", id);
    reads++;
  }
}

int main(int argc, char *argv[]) {
  int i;
  int num = THREADS_MAX;
  pthread_t *reader, *writer;;
  pthread_attr_t attr;
  pthread_mutex_init(&counter_mutex, NULL);
  pthread_mutex_init(&write_mutex, NULL);

  /*
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  */

  reader = malloc(2*num*sizeof(pthread_t));
  /*  if (!reader) checkResults("malloc readers\n", errno); */
  writer = reader + num;

  for (i = 0; i < num; i++) {
    pthread_create(&reader[i], NULL, read_counter, (void *)i);
    pthread_create(&writer[i], NULL, write_counter, (void *)i+num);
  }

  for (i = 0; i < num; i++) {
    pthread_join(reader[i], NULL);
    pthread_join(writer[i], NULL);
  }
  printf ("Main(): Waited on %d threads. Final value of counter = %d. Done.\n", 
          num, counter);
  printf ("Main(): %d reads. %d writes. \n", reads, writes);
  
  pthread_mutex_destroy(&counter_mutex);
  pthread_mutex_destroy(&write_mutex);
  pthread_exit (NULL);

}
