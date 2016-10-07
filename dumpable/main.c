#if 0
BINARY=${0/%.c/}
gcc -Wall -Wextra -std=c11 -pedantic -o $BINARY $0 || exit 1
$PWD/$BINARY
exit
#endif

#define _DEFAULT_SOURCE
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>

void print_dumpable(void)
{
    int r = prctl(PR_GET_DUMPABLE, 0, 0, 0, 0);
    if (r < 0) {
        warn("Cannot get PR_GET_DUMPABLE");
        return;
    }

    printf("%d: PR_GET_DUMPABLE: %d\n", getpid(), r);
}

int main(int argc, char *argv[])
{
    print_dumpable();
    if (argc > 1)
        return EXIT_SUCCESS;

    int r = prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);
    if (r < 0) {
        err(EXIT_FAILURE, "prctl(PR_SET_DUMPABLE, 0, 0, 0, 0)");
    }

    print_dumpable();

    pid_t child = fork();
    if (child < 0) {
        err(EXIT_FAILURE, "fork");
    }

    if (child == 0) {
        char binary[PATH_MAX + 1] = {0};
        const ssize_t r = readlink("/proc/self/exe", binary, sizeof(binary) -1);
        if (r < 0) {
            err(EXIT_FAILURE, "readlink('/proc/self/exe')");
        }
        if ((size_t)r > sizeof(binary) - 1) {
            errx(EXIT_FAILURE, "readlink buffer is too small");
        }

        execl(binary, argv[0], "stop", NULL);
        err(EXIT_FAILURE, "exec");
    }

    waitpid(child, NULL, 0);
    return EXIT_SUCCESS;
}
