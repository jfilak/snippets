#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

void fprintf_dumpable(FILE *stream, const char *label, int r)
{
    fprintf(stream, "%s DUMPABLE : %s\n", label, (r ? "yes" : "no"));
}

int main(int argc, char *argv[])
{
    int r = 0;

    r = prctl(PR_GET_DUMPABLE, 0L, 0L, 0L, 0L);
    if (r < 0) {
        err(EXIT_FAILURE, "Cannot get PR_GET_DUMPABLE");
    }

    printf("/proc/%d\n", getpid());
    fprintf_dumpable(stdout, "Default", r);

    r = seteuid(99);
    if (r < 0) {
        err(EXIT_FAILURE, "Cannot seteuid(99)");
    }

    r = prctl(PR_GET_DUMPABLE, 0L, 0L, 0L, 0L);
    if (r < 0) {
        err(EXIT_FAILURE, "Cannot get PR_GET_DUMPABLE");
    }

    fprintf_dumpable(stdout, "setuid", r);

    r = seteuid(0);
    if (r < 0) {
        err(EXIT_FAILURE, "Cannot seteuid(0)");
    }

    r = prctl(PR_GET_DUMPABLE, 0L, 0L, 0L, 0L);
    if (r < 0) {
        err(EXIT_FAILURE, "Cannot get PR_GET_DUMPABLE");
    }

    fprintf_dumpable(stdout, "re-setuid", r);

    return EXIT_SUCCESS;
}
