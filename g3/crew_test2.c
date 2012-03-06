// magic to make usleep work without warnings
#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREADS_MAX 5
#define ITERATIONS 10

int count = 0;
int readcount = 0;
int writecount = 0;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
pthread_mutex_t w;
pthread_mutex_t r;

void *reader_thread(void *p) {
  long id = (long) p;
  int i = ITERATIONS;
  while (i --> 0) { 
    pthread_mutex_lock(&mutex3);
    pthread_mutex_lock(&r);
    pthread_mutex_lock(&mutex1);
    if (++readcount == 1)
      pthread_mutex_lock(&w);
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&r);
    pthread_mutex_unlock(&mutex3);

    usleep(50);
    printf("Read: thread %ld, counter = %d.\n", id, count);

    pthread_mutex_lock(&mutex1);
    if (--readcount == 0)
      pthread_mutex_unlock(&w);
    pthread_mutex_unlock(&mutex1);
  }
}

void *writer_thread(void *p) {
  long id = (long)p;
  int i = ITERATIONS;
  while (i --> 0) {
    pthread_mutex_lock(&mutex2);
    if (++writecount == 1)
      pthread_mutex_lock(&r);
    pthread_mutex_unlock(&mutex2);

    pthread_mutex_lock(&w);
    usleep(100);
    count++;
    printf("Wrote: thread %ld, counter = %d.\n", id, count);
    pthread_mutex_unlock(&w);

    pthread_mutex_lock(&mutex2);
    if (--writecount == 0)
      pthread_mutex_unlock(&r);
    pthread_mutex_unlock(&mutex2);
  }
}

int main(int argc, char *argv[]) {
  long int i;
  int num = THREADS_MAX;
  pthread_t *reader, *writer;
  pthread_mutex_init(&mutex1, NULL);
  pthread_mutex_init(&mutex2, NULL);
  pthread_mutex_init(&mutex3, NULL);
  pthread_mutex_init(&w, NULL);
  pthread_mutex_init(&r, NULL);

  reader = malloc(2*num*sizeof(pthread_t));
  writer = reader + num;

  for (i = 0; i < num; i++) {
    pthread_create(&reader[i], NULL, reader_thread, (void *)i);
    pthread_create(&writer[i], NULL, writer_thread, (void *)(i+num));
  }
 
  for (i = 0; i < num; i++) {
    pthread_join(reader[i], NULL);
    pthread_join(writer[i], NULL);
  }

  pthread_mutex_destroy(&mutex1);
  pthread_mutex_destroy(&mutex2);
  pthread_mutex_destroy(&mutex3);
  pthread_mutex_destroy(&w);
  pthread_mutex_destroy(&r);

  argc = argc;
  argv = argv;
  return 0;
}
