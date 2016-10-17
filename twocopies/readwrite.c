#include "main.h"

static void xwrite(struct dest *dest, void *buffer, size_t cnt)
{
    size_t t = cnt;
    while (t != 0) {
        int w = write(dest->fd, buffer, cnt);
        if (w < 0) {
            err(EXIT_FAILURE, "Cannot write to %s", dest->path);
        }

        t -= w;
    }
    dest->size += cnt;
}

static void adv(struct dest *dest, off_t offset)
{
    if (lseek(dest->fd, offset, SEEK_CUR) < 0) {
        err(EXIT_FAILURE, "Cannot seek %s", dest->path);
    }
    dest->size += offset;
}

enum op {
    OP_WRITE,
    OP_SEEK,
};

static int write_twice(int in_fd, struct dest *dest_1, struct dest *dest_2)
{
    char buffer[4096];

    enum op last_op = 0;
    for(;;) {
        const int r = read(in_fd, buffer, sizeof(buffer));
        if (r < 0) {
            err(EXIT_FAILURE, "Cannot read input data");
        }

        if (r == 0) {
            if (last_op == OP_SEEK) {
                xwrite(dest_1, "", 1);
                xwrite(dest_2, "", 1);
            }
            break;
        }

        int c = r;
        while(--c >= 0) {
            if (buffer[c] != 0) {
                xwrite(dest_1, buffer, r);
                xwrite(dest_2, buffer, r);
                last_op = OP_WRITE;
                break;
            }
        }

        if (c < 0) {
            adv(dest_1, r);
            adv(dest_2, r);
            last_op = OP_SEEK;
        }
    }

    return EXIT_SUCCESS;
}
