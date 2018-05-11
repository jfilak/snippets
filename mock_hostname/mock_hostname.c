/* Purpose: set an arbitrary hostname in 'docker build'
 * Author: Jakub Filak <filak.jakub@gmail.com>
 * License: GPLv3
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/utsname.h>


typedef int (* t_libc_gethostname)(char *, size_t);
typedef int (* t_libc_uname)(struct utsname *);


int gethostname(char *dest, size_t len)
{
    const char *name = getenv("HOSTNAME");

    if (name == NULL) {
        const t_libc_gethostname libc_gethostname = (t_libc_gethostname)dlsym(RTLD_NEXT, "gethostname");

        if (libc_gethostname == NULL) {
            errno = 0;
            return -1;
        }

        return libc_gethostname(dest, len);
    }

    const char *const dest_end = dest + len;
    for ( ; dest < dest_end && *name != 0; ) {
        *dest++ = *name++;
    }

    if (*name != 0 || dest == dest_end) {
        errno = ENAMETOOLONG;
        return -1;
    }

    *dest = '\0';
    return 0;
}


int uname(struct utsname *buf)
{
    const t_libc_uname libc_uname = (t_libc_uname)dlsym(RTLD_NEXT, "uname");
    if (libc_uname == NULL) {
        errno = 0;
        return -1;
    }

    int r = 0;
    if ((r = libc_uname(buf))) {
        return r;
    } 

    if ((r = gethostname(buf->nodename, sizeof(buf->nodename) / sizeof(buf->nodename[0])))) {
        return r;
    }

    return 0;
}
