#ifndef BUENOS_KERNEL_MUTEX_H
#define BUENOS_KERNEL_MUTEX_H

#include "kernel/spinlock.h"

/*
 *  MUTEX
 */

typedef struct {
  spinlock_t slock;
  int locked;
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

/*
 *  CONDITION VARIABLES.
 */
typedef int cond_t;

/*
 *  Initializes a cond_t structure.
 */
void condition_init(cond_t *cond);

/*
 *  Wait in order to exexute.
 */
void condition_wait(cond_t *cond, lock_t *lock);

/*
 *  Uses signal and continue. A thread calling this function will
 *  continue to execute, and waiting thread are not started undtil
 *  the thread has finished or is waiting. Waiting thread do not
 *  know if the condition is still satisfied, only that it once was.
 */
void condition_signal(cond_t *cond);

/*
 *  Broadcast to all waiting threads, that condition is satisfied.
 */
void condition_broadcast(cond_t *cond);


#endif /* BUENOS_KERNEL_MUTEX_H */
