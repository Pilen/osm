



void *reader_thread(void *p) {

  lock(read);
    while (writers > 0) {
      wait (writers-done, read);
    }

    if (++readcounter == 1)
      lock(writers)
  unlock(read);

  read();

  lock(read);
    if (--readcounter == 1)
      unlock(writers);
  unlock(read);
}

void *writer_thread(void *p) {
  signal(writers-are-here);


  lock(read)
    writers++;

}



void *reader_thread(void *p) {
  pthread_mutex_lock(mutex3);
  pthread_mutex_lock(r);
  pthread_mutex_lock(mutex1);
  if (++readcount ==1)
    pthread_mutex_lock(w);
  pthread_mutex_unlock(mutex1);
  pthread_mutex_unlock(r);
  pthread_mutex_unlock(mutex3);

  usleep(50);
  printf("Read: thread %ld, counter = %d.\n", id, counter);

  pthread_mutex_lock(mutex1);
  if (--readcount == 0)
    pthread_mutex_unlock(w);
  pthread_mutex_unlock(mutex1);
}

void *writer_thread(void *p) {
  

}
