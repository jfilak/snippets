#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <err.h>

struct dest {
    const char *path;
    int fd;
    size_t size;
};

static int xopen_destination(const char *path, struct dest *dest)
{
    unlink(path);
    const int fd = open(path, O_CREAT | O_WRONLY | O_EXCL | O_NOFOLLOW, 0600);
    if (fd < 0) {
        err(EXIT_FAILURE, "Cannot open %s", dest->path);
    }

    dest->path = path;
    return dest->fd = fd;
}

static int write_twice(int in_fd, struct dest *dest_1, struct dest *dest_2);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        errx(EXIT_FAILURE, "Usage:\n %s DESTINATION_PATH_1 DESTINATION_PATH_2", argv[0]);
    }

    struct timespec beg;
    struct timespec end;
    struct dest dest_1 = { 0 };
    struct dest dest_2 = { 0 };
    xopen_destination(argv[1], &dest_1);
    xopen_destination(argv[2], &dest_2);

    int r = clock_gettime(CLOCK_MONOTONIC_RAW, &beg);
    if (r != 0) {
        err(EXIT_FAILURE, "Begin: clock_gettime");
    }

    write_twice(STDIN_FILENO, &dest_1, &dest_2);

    r = clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    if (r != 0) {
        err(EXIT_FAILURE, "End: clock_gettime");
    }

    long s  = end.tv_sec  - beg.tv_sec;
    long ns = end.tv_nsec - beg.tv_nsec;
    if (ns <= 0) {
        s  -= 1;
        ns += 1000000000L;
    }

    printf("%s: %zu\n", dest_1.path, dest_1.size);
    printf("%s: %zu\n", dest_2.path, dest_2.size);
    printf("Operation took: %lds %ldns\n", s, ns);

    if (dest_2.fd >= 0) {
        close(dest_2.fd);
    }

    if (dest_1.fd >= 0) {
        close(dest_1.fd);
    }

    return r;
}
