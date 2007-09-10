/*
 * Copyright (C) 2006 Google. All Rights Reserved.
 * Amit Singh <singh@>
 */

#ifndef _MACFUSE_H_
#define _MACFUSE_H_

/* XXX: should be including <common/*.h> from MacFUSE kernel source. */

#include <sys/ioctl.h>

/* FUSEDEVIOCxxx */

/* Get mounter's pid. */
#define FUSEDEVGETMOUNTERPID           _IOR('F', 1,  u_int32_t)

/* Check if FUSE_INIT kernel-user handshake is complete. */
#define FUSEDEVIOCGETHANDSHAKECOMPLETE _IOR('F', 2,  u_int32_t)

/* Mark the daemon as dead. */
#define FUSEDEVIOCSETDAEMONDEAD        _IOW('F', 3,  u_int32_t)

/* Tell the kernel which operations the daemon implements. */
#define FUSEDEVIOCSETIMPLEMENTEDBITS   _IOW('F', 4,  u_int64_t)

#define FUSE_DEV_TRUNK "/dev/fuse"
#define LOAD_PROG      "/Library/Filesystems/fusefs.fs/Support/load_fusefs"
#define MOUNT_PROG     "/Library/Filesystems/fusefs.fs/Support/mount_fusefs"

#define NFUSEDEVICE    16

#endif /* _MACFUSE_H_ */
