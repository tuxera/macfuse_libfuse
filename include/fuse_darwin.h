/*
 * Copyright (C) 2006,2007 Google. All Rights Reserved.
 * Amit Singh <singh@>
 */

#if (__FreeBSD__ >= 10)

#ifndef _FUSE_DARWIN_H_
#define _FUSE_DARWIN_H_

#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>

/* Semaphores */

struct __local_sem_t
{
    unsigned int    count;
    pthread_mutex_t count_lock;
    pthread_cond_t  count_cond;
};

typedef struct fuse_sem {
    int id;
    union {
        struct __local_sem_t local;
    } __data;
} fuse_sem_t;

#define FUSE_SEM_VALUE_MAX ((int32_t)32767)

int fuse_sem_init(fuse_sem_t *sem, int pshared, unsigned int value);
int fuse_sem_destroy(fuse_sem_t *sem);
int fuse_sem_getvalue(fuse_sem_t *sem, unsigned int *value);
int fuse_sem_post(fuse_sem_t *sem);
int fuse_sem_timedwait(fuse_sem_t *sem, const struct timespec *abs_timeout);
int fuse_sem_trywait(fuse_sem_t *sem);
int fuse_sem_wait(fuse_sem_t *sem);

#ifdef DARWIN_SEMAPHORE_COMPAT

/* Caller must not include <semaphore.h> */

typedef fuse_sem_t sem_t;

#define sem_init(s, p, v)   fuse_sem_init(s, p, v)
#define sem_destroy(s)      fuse_sem_destroy(s)
#define sem_getvalue(s, v)  fuse_sem_getvalue(s, v)
#define sem_post(s)         fuse_sem_post(s)
#define sem_timedwait(s, t) fuse_sem_timedwait(s, t)
#define sem_trywait(s)      fuse_sem_trywait(s)
#define sem_wait(s)         fuse_sem_wait(s)

#define SEM_VALUE_MAX       FUSE_SEM_VALUE_MAX

#endif /* DARWIN_SEMAPHORE_COMPAT */

/* Notifications */

#define LIBFUSE_BUNDLE_IDENTIFIER "com.google.filesystems.libfuse"

#define LIBFUSE_UNOTIFICATIONS_OBJECT                 \
    LIBFUSE_BUNDLE_IDENTIFIER ".unotifications"

#define LIBFUSE_UNOTIFICATIONS_NOTIFY_OSISTOONEW      \
    LIBFUSE_BUNDLE_IDENTIFIER ".osistoonew"

#define LIBFUSE_UNOTIFICATIONS_NOTIFY_OSISTOOOLD      \
    LIBFUSE_BUNDLE_IDENTIFIER ".osistooold"                    
 
#define LIBFUSE_UNOTIFICATIONS_NOTIFY_VERSIONMISMATCH \
    LIBFUSE_BUNDLE_IDENTIFIER ".versionmismatch"

/* Versioning */

const char *macfuse_version(void);

/* Kernel */

#ifdef DARWIN_KERNEL_COMMON

#include <fuse_param.h>
#include <fuse_ioctl.h>
#include <fuse_version.h>

#endif /* DARWIN_KERNEL_COMMON */

int  fuse_knote_path_np(const char *path, uint32_t note);
int  fuse_purge_path_np(const char *path);
int  fuse_purge_path_set_size_np(const char *path, off_t newsize);
long fuse_os_version_major(void);

#endif /* _FUSE_DARWIN_H_ */

#endif /* __FreeBSD__ >= 10 */
