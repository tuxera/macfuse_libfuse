/*
 * Copyright (C) 2006-2008 Google. All Rights Reserved.
 * Amit Singh <singh@>
 */

#if (__FreeBSD__ >= 10)

#ifndef _FUSE_DARWIN_PRIVATE_H_
#define _FUSE_DARWIN_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fuse_lowlevel.h"
#include "fuse_darwin.h"

#include <fuse_param.h>
#include <fuse_ioctl.h>
#include <fuse_version.h>

#ifdef __cplusplus
}
#endif

fuse_ino_t fuse_lookup_inode_internal_np(const char *path);
int        fuse_resize_node_internal_np(const char *path, off_t newsize);

#endif /* _FUSE_DARWIN_PRIVATE_H_ */

#endif /* __FreeBSD__ >= 10 */
