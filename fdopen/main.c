#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

void test(const char *filename, int fdmode, const char *fdmodestr, const char *filemode)
{
    const int fd = open(filename, fdmode);
    if (fd < 0 ) {
        err(EXIT_FAILURE, "open(%s, %s)", filename, fdmodestr);
    }

    errno = 0;
    FILE *const f = fdopen(fd, filemode);
    printf("fdopen(open(%s), %-8s), '%-2s') : %s\n", filename, fdmodestr, filemode, strerror(errno));
    if (f == NULL) {
        close(fd);
    }
    else {
        fclose(f);
    }
}

#define make_test(filename, fdmode, filemode) \
    test(filename, fdmode, #fdmode, filemode);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        errx(EXIT_FAILURE, "Pass a file as the first argument");
    }

    const char *modes[] = { "a", "a+", "r", "r+", "w", "w+", NULL };

    for (const char **m = modes; *m; ++m) {
        make_test(argv[1], O_RDWR,   *m);
        make_test(argv[1], O_WRONLY, *m);
        make_test(argv[1], O_RDONLY, *m);
    }

    return 0;
}
