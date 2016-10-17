#include "main.h"

#include <limits.h>

static ssize_t splice_full(int in_fd, int out_fd, size_t size)
{
    ssize_t total = 0;
    while (size != 0) {
        const ssize_t b = splice(in_fd, NULL, out_fd, NULL, size, 0);
        if (b < 0) {
            return b;
        }

        if (b == 0) {
            break;
        }

        total += b;
        size -= b;
    }

    return total;
}

static void xsplice_full(int in_fd, struct dest *dest, size_t size)
{
    const ssize_t spliced = splice_full(in_fd, dest->fd, size);
    if (spliced < 0) {
        err(EXIT_FAILURE, "Failed to write data to file %s", dest->path);
    }
    dest->size += spliced;
}

static int write_twice(int in_fd, struct dest *dest_1, struct dest *dest_2)
{
    int scp[2];
    if (pipe(scp) < 0) {
        err(EXIT_FAILURE, "pipe");
    }

    for (;;) {
        const ssize_t to_write = tee(in_fd, scp[1], INT_MAX, 0);
        if (to_write < 0) {
            err(EXIT_FAILURE, "tee");
        }

        if (to_write == 0) {
            break;
        }

        xsplice_full(in_fd, dest_1, to_write);
        xsplice_full(scp[0], dest_2, to_write);
    }

    close(scp[0]);
    close(scp[1]);

    return EXIT_SUCCESS;
}
