#include "locks.h"

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
