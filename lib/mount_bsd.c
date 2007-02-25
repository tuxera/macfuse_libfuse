/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2005-2006 Csaba Henk <csaba.henk@creo.hu>

    This program can be distributed under the terms of the GNU LGPL.
    See the file COPYING.LIB.
*/

#if (__FreeBSD__ >= 10)
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#endif

#include "fuse_i.h"
#include "fuse_opt.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <paths.h>

#if (__FreeBSD__ >= 10)
#define FUSERMOUNT_PROG  "/System/Library/Filesystems/fusefs.fs/mount_fusefs"
#define FUSE_DEV_TRUNK   "/dev/fuse"
#define PRIVATE_LOAD_COMMAND "/Library/Extensions/fusefs.kext/Contents/Resources/load_fusefs"

#include <sys/param.h>
#include <sys/mount.h>
#include <AssertMacros.h>

static const char *MacFUSE = "MacFUSE version 0.2.2, " __DATE__ ", " __TIME__;

static int
checkloadable(void)
{
    int ret;
    struct vfsconf vfc;
    
    ret = getvfsbyname("fusefs", &vfc);
    
    return ret;
}   

int
loadkmod()
{
    int result = -1;
    int pid, terminated_pid;
    union wait status;

    pid = fork();

    if (pid == 0) {
        result = execl(PRIVATE_LOAD_COMMAND, PRIVATE_LOAD_COMMAND, NULL);
        
        /* exec failed */
        goto Return;
    }

    require_action(pid != -1, Return, result = errno);

    while ((terminated_pid = wait4(pid, (int *)&status, 0, NULL)) < 0) {
        /* retry if EINTR, else break out with error */
        if ( errno != EINTR ) {
            break;
        }
    }

    if ((terminated_pid == pid) && (WIFEXITED(status))) {
        result = WEXITSTATUS(status);
    } else {
        result = -1;
    }

Return:
    check_noerr_string(result, strerror(errno));
    
    return result;
}

#else
#define FUSERMOUNT_PROG         "mount_fusefs"
#define FUSE_DEV_TRUNK          "/dev/fuse"
#endif

enum {
    KEY_ALLOW_ROOT,
    KEY_RO,
    KEY_HELP,
    KEY_VERSION,
    KEY_KERN
};

struct mount_opts {
    int allow_other;
    int allow_root;
    int ishelp;
    char *kernel_opts;
};

static const struct fuse_opt fuse_mount_opts[] = {
    { "allow_other", offsetof(struct mount_opts, allow_other), 1 },
    { "allow_root", offsetof(struct mount_opts, allow_root), 1 },
    FUSE_OPT_KEY("allow_root",          KEY_ALLOW_ROOT),
    FUSE_OPT_KEY("-r",                  KEY_RO),
    FUSE_OPT_KEY("-h",                  KEY_HELP),
    FUSE_OPT_KEY("--help",              KEY_HELP),
    FUSE_OPT_KEY("-V",                  KEY_VERSION),
    FUSE_OPT_KEY("--version",           KEY_VERSION),
    /* standard FreeBSD mount options */
    FUSE_OPT_KEY("dev",                 KEY_KERN),
    FUSE_OPT_KEY("async",               KEY_KERN),
    FUSE_OPT_KEY("atime",               KEY_KERN),
    FUSE_OPT_KEY("dev",                 KEY_KERN),
    FUSE_OPT_KEY("exec",                KEY_KERN),
    FUSE_OPT_KEY("suid",                KEY_KERN),
    FUSE_OPT_KEY("symfollow",           KEY_KERN),
    FUSE_OPT_KEY("rdonly",              KEY_KERN),
    FUSE_OPT_KEY("sync",                KEY_KERN),
    FUSE_OPT_KEY("union",               KEY_KERN),
    FUSE_OPT_KEY("userquota",           KEY_KERN),
    FUSE_OPT_KEY("groupquota",          KEY_KERN),
    FUSE_OPT_KEY("clusterr",            KEY_KERN),
    FUSE_OPT_KEY("clusterw",            KEY_KERN),
    FUSE_OPT_KEY("suiddir",             KEY_KERN),
    FUSE_OPT_KEY("snapshot",            KEY_KERN),
    FUSE_OPT_KEY("multilabel",          KEY_KERN),
    FUSE_OPT_KEY("acls",                KEY_KERN),
    FUSE_OPT_KEY("force",               KEY_KERN),
    FUSE_OPT_KEY("update",              KEY_KERN),
    FUSE_OPT_KEY("ro",                  KEY_KERN),
    FUSE_OPT_KEY("rw",                  KEY_KERN),
    FUSE_OPT_KEY("auto",                KEY_KERN),
    /* options supported under both Linux and FBSD */
    FUSE_OPT_KEY("allow_other",         KEY_KERN),
    FUSE_OPT_KEY("default_permissions", KEY_KERN),
    /* FBSD FUSE specific mount options */
    FUSE_OPT_KEY("private",             KEY_KERN),
    FUSE_OPT_KEY("neglect_shares",      KEY_KERN),
    FUSE_OPT_KEY("push_symlinks_in",    KEY_KERN),
    /* stock FBSD mountopt parsing routine lets anything be negated... */
    FUSE_OPT_KEY("nodev",               KEY_KERN),
    FUSE_OPT_KEY("noasync",             KEY_KERN),
    FUSE_OPT_KEY("noatime",             KEY_KERN),
    FUSE_OPT_KEY("nodev",               KEY_KERN),
    FUSE_OPT_KEY("noexec",              KEY_KERN),
    FUSE_OPT_KEY("nosuid",              KEY_KERN),
    FUSE_OPT_KEY("nosymfollow",         KEY_KERN),
    FUSE_OPT_KEY("nordonly",            KEY_KERN),
    FUSE_OPT_KEY("nosync",              KEY_KERN),
    FUSE_OPT_KEY("nounion",             KEY_KERN),
    FUSE_OPT_KEY("nouserquota",         KEY_KERN),
    FUSE_OPT_KEY("nogroupquota",        KEY_KERN),
    FUSE_OPT_KEY("noclusterr",          KEY_KERN),
    FUSE_OPT_KEY("noclusterw",          KEY_KERN),
    FUSE_OPT_KEY("nosuiddir",           KEY_KERN),
    FUSE_OPT_KEY("nosnapshot",          KEY_KERN),
    FUSE_OPT_KEY("nomultilabel",        KEY_KERN),
    FUSE_OPT_KEY("noacls",              KEY_KERN),
    FUSE_OPT_KEY("noforce",             KEY_KERN),
    FUSE_OPT_KEY("noupdate",            KEY_KERN),
    FUSE_OPT_KEY("noro",                KEY_KERN),
    FUSE_OPT_KEY("norw",                KEY_KERN),
    FUSE_OPT_KEY("noauto",              KEY_KERN),
    FUSE_OPT_KEY("noallow_other",       KEY_KERN),
    FUSE_OPT_KEY("nodefault_permissions", KEY_KERN),
    FUSE_OPT_KEY("noprivate",           KEY_KERN),
    FUSE_OPT_KEY("noneglect_shares",    KEY_KERN),
    FUSE_OPT_KEY("nopush_symlinks_in",  KEY_KERN),
#if (__FreeBSD__ >= 10)
    /* Mac OS X options */
    FUSE_OPT_KEY("blocksize=",          KEY_KERN),
    FUSE_OPT_KEY("daemon_timeout=",     KEY_KERN),
    FUSE_OPT_KEY("extended_security",   KEY_KERN),
    FUSE_OPT_KEY("fsid=",               KEY_KERN),
    FUSE_OPT_KEY("fsname=",             KEY_KERN),
    FUSE_OPT_KEY("init_timeout=",       KEY_KERN),
    FUSE_OPT_KEY("iosize=",             KEY_KERN),
    FUSE_OPT_KEY("jail_symlinks",       KEY_KERN),
    FUSE_OPT_KEY("noapplespecial",      KEY_KERN),
    FUSE_OPT_KEY("noattrcache",         KEY_KERN),
    FUSE_OPT_KEY("noauthopaque",        KEY_KERN),
    FUSE_OPT_KEY("noauthopaqueaccess",  KEY_KERN),
    FUSE_OPT_KEY("nobrowse",            KEY_KERN),
    FUSE_OPT_KEY("nolocalcaches",       KEY_KERN),
    FUSE_OPT_KEY("noreadahead",         KEY_KERN),
    FUSE_OPT_KEY("nosynconclose",       KEY_KERN),
    FUSE_OPT_KEY("nosyncwrites",        KEY_KERN),
    FUSE_OPT_KEY("noubc",               KEY_KERN),
    FUSE_OPT_KEY("novncache",           KEY_KERN),
    FUSE_OPT_KEY("ping_diskarb",        KEY_KERN),
    FUSE_OPT_KEY("subtype=",            KEY_KERN),
    FUSE_OPT_KEY("volname=",            KEY_KERN),
#else
    /* Linux specific mount options, but let just the mount util handle them */
    FUSE_OPT_KEY("fsname=",             KEY_KERN),
    FUSE_OPT_KEY("nonempty",            KEY_KERN),
    FUSE_OPT_KEY("large_read",          KEY_KERN),
    FUSE_OPT_KEY("max_read=",           KEY_KERN),
#endif
    FUSE_OPT_END
};

static void mount_help(void)
{
    fprintf(stderr,
            "    -o allow_root          allow access to root\n"
            );
    system(FUSERMOUNT_PROG " --help");
    fputc('\n', stderr);
}

static void mount_version(void)
{
    system(FUSERMOUNT_PROG " --version");
}

static int fuse_mount_opt_proc(void *data, const char *arg, int key,
                               struct fuse_args *outargs)
{
    struct mount_opts *mo = data;

    switch (key) {
    case KEY_ALLOW_ROOT:
        if (fuse_opt_add_opt(&mo->kernel_opts, "allow_other") == -1 ||
            fuse_opt_add_arg(outargs, "-oallow_root") == -1)
            return -1;
        return 0;

    case KEY_RO:
        arg = "ro";
        /* fall through */

    case KEY_KERN:
        return fuse_opt_add_opt(&mo->kernel_opts, arg);

    case KEY_HELP:
        mount_help();
        mo->ishelp = 1;
        break;

    case KEY_VERSION:
        mount_version();
        mo->ishelp = 1;
        break;
    }
    return 1;
}

void fuse_unmount_compat22(const char *mountpoint)
{
    char dev[128];
    char *ssc, *umount_cmd;
    FILE *sf;
    int rv;
    char *seekscript =
    "exec 2>/dev/null; " /* error message is annoying in help output */
    "/usr/bin/fstat " FUSE_DEV_TRUNK "* | "
    "/usr/bin/awk 'BEGIN{ getline; if (! ($3 == \"PID\" && $10 == \"NAME\")) exit 1; }; "
    "              { if ($3 == %d) print $10; }' | "
    "/usr/bin/sort | "
    "/usr/bin/uniq | "
    "/usr/bin/awk '{ i += 1; if (i > 1){ exit 1; }; printf; }; END{ if (i == 0) exit 1; }'";

    (void) mountpoint;

    /*
     * If we don't know the fd, we have to resort to the scripted solution --
     * iterating over the fd-s is unpractical, as we don't know how many of
     * open files we have. (This could be looked up in procfs -- however,
     * that's optional on FBSD; or read out from the kmem --  however, that's
     * bound to privileges (in fact, that's what happens when we call the
     * setgid kmem fstat(1) utility).
     */
    asprintf(&ssc, seekscript, getpid());

    errno = 0;
    sf = popen(ssc, "r");
    if (! sf)
        return;

    fgets(dev, sizeof(dev), sf);
    rv = pclose(sf);
    if (rv)
        return;

#if (__FreeBSD__ >= 10)
    {
        int ret;
        char *rp = NULL;
        char resolved_path[PATH_MAX];

        rp = realpath(mountpoint, resolved_path);
        if (rp) {
            ret = unmount(resolved_path, 0);
        }
    }
#else
    asprintf(&umount_cmd, "/sbin/umount %s", dev);
    system(umount_cmd);
#endif
}

void fuse_kern_unmount(const char *mountpoint, int fd)
{
    char *ep, *umount_cmd, dev[128];
    struct stat sbuf;

    (void)mountpoint;

    if (fstat(fd, &sbuf) == -1)
        return;

    devname_r(sbuf.st_rdev, S_IFCHR, dev, 128);

    if (strncmp(dev, "fuse", 4))
        return;

    strtol(dev + 4, &ep, 10);
    if (*ep != '\0')
        return;

#if (__FreeBSD__ >= 10)
    {
        int ret;
        char *rp = NULL;
        char resolved_path[PATH_MAX];

        rp = realpath(mountpoint, resolved_path);
        if (rp) {
            ret = unmount(resolved_path, 0);
        }
    }
#else
    asprintf(&umount_cmd, "/sbin/umount " _PATH_DEV "%s", dev);
    system(umount_cmd);
#endif
}

/* Check if kernel is doing init in background */
static int init_backgrounded(void)
{
#if (__FreeBSD__ >= 10)
    return 0;
#else
    int ibg, len;

    len = sizeof(ibg);

    if (sysctlbyname("vfs.fuse.init_backgrounded", &ibg, &len, NULL, 0))
        return 0;

    return ibg;
#endif
}


static int fuse_mount_core(const char *mountpoint, const char *opts)
{
    const char *mountprog = FUSERMOUNT_PROG;
    int fd;
    char *fdnam, *dev;
    int pid;

#if (__FreeBSD__ >= 10)
    if (checkloadable()) {
        if (loadkmod()) {
            fprintf(stderr, "fusefs file system is not available");
            return -1;
        }
    }
#endif

    fdnam = getenv("FUSE_DEV_FD");

    if (fdnam) {
        char *ep;

        fd = strtol(fdnam, &ep, 10);

        if (*ep != '\0') {
            fprintf(stderr, "invalid value given in FUSE_DEV_FD\n");
            return -1;
        }

        if (fd < 0)
            return -1;

        goto mount;
    }

    dev = getenv("FUSE_DEV_NAME");

    if (dev) {
        if ((fd = open(dev, O_RDWR)) < 0) {
            perror("fuse: failed to open fuse device");
            return -1;
        }
    } else {
#define NFUSEDEVICE 16
        int r, devidx = -1;
        char devpath[MAXPATHLEN];

        for (r = 0; r < NFUSEDEVICE; r++) {
            snprintf(devpath, MAXPATHLEN - 1, "/dev/fuse%d", r);
            fd = open(devpath, O_RDWR);
            if (fd >= 0) {
                dev = devpath;
                devidx = r;
                break;
            }
        }
        if (devidx == -1) {
            perror("fuse: failed to open fuse device");
            return -1;
        }
    }

mount:
    if (getenv("FUSE_NO_MOUNT") || ! mountpoint)
        goto out;

    pid = fork();

    if (pid == -1) {
        perror("fuse: fork() failed");
        close(fd);
        return -1;
    }

    if (pid == 0) {
        if (! init_backgrounded()) {
            /*
             * If init is not backgrounded, we have to call the mount util
             * backgrounded, to avoid deadlock.
             */

            pid = fork();
    
            if (pid == -1) {
                perror("fuse: fork() failed");
                close(fd);
                exit(1);
            }
        }

        if (pid == 0) {
            const char *argv[32];
            int a = 0;

            if (! fdnam)
                asprintf(&fdnam, "%d", fd);

            argv[a++] = mountprog;
            if (opts) {
                argv[a++] = "-o";
                argv[a++] = opts;
            }
            argv[a++] = fdnam;
            argv[a++] = mountpoint;
            argv[a++] = NULL;
            execvp(mountprog, (char **) argv);
            perror("fuse: failed to exec mount program");
            exit(1);
        }
#if (__FreeBSD__ >= 10)
        else {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && (WEXITSTATUS(status) != 0)) {
                exit(WEXITSTATUS(status));
            }
        }
#endif

        exit(0);
    }

#if (__FreeBSD__ >= 10)
    {
        int status;

        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && (WEXITSTATUS(status) != 0)) {
            exit(WEXITSTATUS(status));
        }
    }
#else
    waitpid(pid, NULL, 0);
#endif

out:
    return fd;
}

int fuse_kern_mount(const char *mountpoint, struct fuse_args *args)
{
    struct mount_opts mo;
    int res = -1;

    memset(&mo, 0, sizeof(mo));
    /* mount util should not try to spawn the daemon */
    setenv("MOUNT_FUSEFS_SAFE", "1", 1);
    /* to notify the mount util it's called from lib */
    setenv("MOUNT_FUSEFS_CALL_BY_LIB", "1", 1);

    if (args &&
        fuse_opt_parse(args, &mo, fuse_mount_opts, fuse_mount_opt_proc) == -1)
        return -1;

    if (mo.allow_other && mo.allow_root) {
        fprintf(stderr, "fuse: 'allow_other' and 'allow_root' options are mutually exclusive\n");
        goto out;
    }
    if (mo.ishelp)
        return 0;

    res = fuse_mount_core(mountpoint, mo.kernel_opts);
 out:
    free(mo.kernel_opts);
    return res;
}

__asm__(".symver fuse_unmount_compat22,fuse_unmount@FUSE_2.2");
