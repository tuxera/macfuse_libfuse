/*
 * Copyright (C) 2006,2007 Google. All Rights Reserved.
 * Amit Singh <singh@>
 */

#include <errno.h>
#include <sys/types.h>
#include <pthread.h>

#include "fuse_darwin.h"

/* Half-baked makeshift implementation (cancelable). */

/* http://www.opengroup.org/onlinepubs/007908799/xsh/sem_init.html */
int
fuse_sem_init(fuse_sem_t *sem, int pshared, int value)
{

    if (value > FUSE_SEM_VALUE_MAX) {
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

    sem->value = FUSE_SEM_VALUE_DEAD;

    pthread_mutex_unlock(&sem->mutex);
    pthread_mutex_destroy(&sem->mutex);

    return 0;
}

/* http://www.opengroup.org/onlinepubs/007908799/xsh/sem_wait.html */
int
fuse_sem_wait(fuse_sem_t *sem)
{
    if (!sem || (sem->value == FUSE_SEM_VALUE_DEAD)) {
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
int
fuse_sem_post(fuse_sem_t *sem)
{
    if (!sem || (sem->value == FUSE_SEM_VALUE_DEAD)) {
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
