/*
 * Copyright (C) 2006,2007 Google. All Rights Reserved.
 * Amit Singh <singh@>
 */

#ifndef _DARWIN_COMPAT_H_
#define _DARWIN_COMPAT_H_

#include <AvailabilityMacros.h>

#ifdef MAC_OS_X_VERSION_10_5

/* 10.5+ */

/* Half-baked makeshift implementation (cancelable). */

#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
    
typedef struct {
    int32_t         value;
    int32_t         wakeups;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} fuse_sem_t;

typedef fuse_sem_t sem_t;
        
#define SEM_VALUE_DEAD ((int32_t)0xdeadbeef)
#define SEM_VALUE_MAX  ((int32_t)32767)

/* http://www.opengroup.org/onlinepubs/007908799/xsh/sem_init.html */
static __inline__
int
fuse_sem_init(fuse_sem_t *sem, int pshared, int value)
{

    if (value > SEM_VALUE_MAX) {
        errno = EINVAL;
        return -1;
    }

    if (pshared) {
        errno = ENOSYS;
        return -1;
    }

    if (!sem) {
        errno = EINVAL;
        return -1;
    }

    sem->value = value;
    sem->wakeups = 0;
    pthread_cond_init(&sem->cond, NULL);
    pthread_mutex_init(&sem->mutex, NULL);

    return 0;
}

/* http://www.opengroup.org/onlinepubs/007908799/xsh/sem_destroy.html */
static __inline__
int
fuse_sem_destroy(fuse_sem_t *sem)
{
    int ret;

    if (!sem) {
        errno = EINVAL;
        return -1;
    }

    ret = pthread_mutex_lock(&sem->mutex);
    if (ret) {
        errno = ret;
        return -1;
    }

    if (sem->value < 0) {
        pthread_mutex_unlock(&sem->mutex);
        errno = EBUSY;
        return -1;
    }

    pthread_cond_destroy(&sem->cond);

    sem->value = SEM_VALUE_DEAD;

    pthread_mutex_unlock(&sem->mutex);
    pthread_mutex_destroy(&sem->mutex);

    return 0;
}

/* http://www.opengroup.org/onlinepubs/007908799/xsh/sem_wait.html */
static __inline__
int
fuse_sem_wait(fuse_sem_t *sem)
{
    if (!sem || (sem->value == SEM_VALUE_DEAD)) {
        errno = EINVAL;
        return -1;
    }

    pthread_mutex_lock(&sem->mutex);
    sem->value--;
    if (sem->value < 0) {
        do {
            /*
             * XXX: This is not really supposed to fail except for some EINVALs
             *      that shouldn't apply here.
             */
            (void)pthread_cond_wait(&sem->cond, &sem->mutex);
        } while (sem->wakeups < 1);
        sem->wakeups--;
    }
    pthread_mutex_unlock(&sem->mutex);

    return 0;
}

/* http://www.opengroup.org/onlinepubs/007908799/xsh/sem_post.html */
static __inline__
int
fuse_sem_post(fuse_sem_t *sem)
{
    if (!sem || (sem->value == SEM_VALUE_DEAD)) {
        errno = EINVAL;
        return -1;
    }

    pthread_mutex_lock(&sem->mutex);
    sem->value++;
    if (sem->value <= 0) {
        sem->wakeups++;
        (void)pthread_cond_signal(&sem->cond);
    }
    pthread_mutex_unlock(&sem->mutex);

    return 0;
}

#define sem_init(s, a, b) fuse_sem_init(s, a, b)
#define sem_destroy(s)    fuse_sem_destroy(s)
#define sem_post(s)       fuse_sem_post(s)
#define sem_wait(s)       fuse_sem_wait(s)

#else

/* 10.4- */

#include <mach/mach.h>

#define sem_init(s, a, b) semaphore_create(mach_task_self(), (s), \
                                           SYNC_POLICY_FIFO, 0)
#define sem_destroy(s)    semaphore_destroy(mach_task_self(), (semaphore_t)*(s))
#define sem_post(s)       semaphore_signal((semaphore_t)*(s))
#define sem_wait(s)       semaphore_wait((semaphore_t)*(s))

#endif /* MAC_OS_X_VERSION_10.5 */

#endif /* _DARWIN_COMPAT_H_ */
