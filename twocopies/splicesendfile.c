#include "main.h"

#include <limits.h>
#include <inttypes.h>
#include <sys/sendfile.h>

static ssize_t splice_entire(int in_fd, struct dest *dest)
{
    for (;;) {
        const ssize_t b = splice(in_fd, NULL, dest->fd, NULL, UINT_MAX, 0);
        if (b < 0) {
            return b;
        }

        if (b == 0) {
            break;
        }

        dest->size += b;
    }

    return 0;
}

static ssize_t sendfile_entire(int in_fd, struct dest *dest)
{
    for (;;) {
        const ssize_t b = sendfile(dest->fd, in_fd, NULL, UINT_MAX);
        if (b < 0) {
            return b;
        }

        if (b == 0) {
            break;
        }

        dest->size += b;
    }

    return 0;
}

static int write_twice(int in_fd, struct dest *dest_1, struct dest *dest_2)
{
    ssize_t r = splice_entire(in_fd, dest_1);
    if (r < 0) {
        err(EXIT_FAILURE, "Cannot write to %s", dest_1->path);
    }

    close(dest_1->fd);
    dest_1->fd = open(dest_1->path, O_RDONLY);
    if (dest_1->fd < 0) {
        err(EXIT_FAILURE, "open(%s, O_RDONLY)", dest_1->path);
    }

    r = sendfile_entire(dest_1->fd, dest_2);
    if (r < 0) {
        err(EXIT_FAILURE, "sendfile(%s, %s)", dest_2->path, dest_1->path);
    }

    return EXIT_SUCCESS;
}
