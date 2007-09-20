/*
 * Copyright (C) 2006,2007 Google. All Rights Reserved.
 * Amit Singh <singh@>
 */

#if (__FreeBSD__ >= 10)

#ifndef _FUSE_DARWIN_H_
#define _FUSE_DARWIN_H_

/* POSIX Semaphore */

typedef struct fuse_sem {
    int32_t         value;
    int32_t         wakeups;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} fuse_sem_t;

#define FUSE_SEM_VALUE_MAX  ((int32_t)32767)
#define FUSE_SEM_VALUE_DEAD ((int32_t)0xdeadbeef)

int fuse_sem_init(fuse_sem_t *sem, int pshared, int value);
int fuse_sem_destroy(fuse_sem_t *sem);
int fuse_sem_post(fuse_sem_t *sem);
int fuse_sem_wait(fuse_sem_t *sem);

#ifdef DARWIN_SEMAPHORE_COMPAT

typedef fuse_sem_t sem_t;

#define sem_init(s, a, b) fuse_sem_init(s, a, b)
#define sem_destroy(s)    fuse_sem_destroy(s)
#define sem_post(s)       fuse_sem_post(s)
#define sem_wait(s)       fuse_sem_wait(s)

#define SEM_VALUE_MAX     FUSE_SEM_VALUE_MAX

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

/* Kernel */

#ifdef DARWIN_KERNEL_COMMON

#include <fuse_param.h>
#include <fuse_ioctl.h>
#include <fuse_version.h>

#endif /* DARWIN_KERNEL_COMMON */

#endif /* _FUSE_DARWIN_H_ */

#endif /* __FreeBSD__ >= 10 */
