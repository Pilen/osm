#include "kernel/lock_cond.h"
#include "kernel/spinlock.h"
#include "kernel/sleepq.h"
#include "kernel/thread.h"

/*
 *  MUTEX.
 */

/*
 *  Initialize an already allocated lock_t structure. Return 0 on 
 *  success, and negative value on failure.
 */
int lock_reset(lock_t *lock) {
  if (lock == NULL)
    return -1;
  lock->locked = 0;
  spinlock_reset(lock->slock);
  return 0;
}

/*
 *  Acquire lock. Uses sleepq for waiting.
 */
void lock_acquire(lock_t *lock) {
  interrupt_status_t intr_status;
  intr_status = _interrupt_disable();
  spinlock_acquire(lock->slock);
  
  while (lock->locked == 1) {
    sleepq_add(lock);
    spinlock_release(lock->slock);
    thread_switch();
    spinlock_aquire(lock->slock);
  }
  
  lock->locked = 1;
  spinlock_release(lock->slock);
  _interrupt_set_state(intr_status);
}

/*
 *  Release lock.
 */
void lock_release(lock_t *lock) {
  interrupt_status_t intr_status;
  intr_status = _interrupt_disable();
  spinlock_acquire(lock->slock);
  lock->locked = 0;
  sleepq_wake(lock);
  spinlock_release(lock->slock);
  _interrupt_set_state(intr_status);
}


/*
 *  CONDITION VARIABLES.
 */

/*
 *  Initializes cond_t.
 */
void condition_init(cond_t *cond){
  *cond = 1337;
}

/*
 *  Wait in order to exexute. Asserts that caller has remembered
 *  to lock the mutex, before calling condition_wait().
 */
void condition_wait(cond_t *cond, lock_t *lock){
  interrupt_status_t intr_status;
  intr_status = _interrupt_disable();
  /* måske spinlock */
  KERNEL_ASSERT(lock->locked != 0);

  sleepq_add(cond);
  lock_release(lock);
  thread_switch();
  lock_aquire(lock);
  _interrupt_set_state(intr_status);
}

/*
 *  Uses signal and continue. A thread calling this function will
 *  continue to execute, and waiting thread are not started undtil
 *  the thread has finished or is waiting. Waiting thread do not
 *  know if condition is still satisfied, only that it once was.
 */
void condition_signal(cond_t *cond){
  sleepq_wake(cond);
}

/*
 *  Broadcast to all waiting threads, that condition is satisfied.
 */
void condition_broadcast(cond_t *cond){
  sleepq_wake_all(cond);
}
