#ifndef BUENOS_KERNEL_MUTEX_H
#define BUENOS_KERNEL_MUTEX_H

#include "kernel/spinlock.h"
#include "kernel/thread.h"

typedef struct {
  spinlock_t slock;
  int value;
  TID_t creator;
} lock_t;

/*
 *  Initialize an already allocated lock_t structure. Return 0 on 
 *  success, and negative value on failure.
 */
int lock_reset(lock_t *lock);

/*
 *  Acquire lock. Uses sleepq for waiting.
 */
void lock_acquire(lock_t *lock);

/*
 *  Release lock.
 */
void lock_release(lock_t *lock);

#endif /* BUENOS_KERNEL_MUTEX_H */
