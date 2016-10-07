#if 0
gcc -Wall -Wextra -std=c11 -pedantic -o ${0%.c} $0
exit
#endif

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    long bufsize = 3072*1024;
    long loops = 1024;
    bool clean = false;
    bool random = false;
    int opt;
    while ((opt = getopt(argc, argv, "rcb:l:")) != -1) {
        switch (opt) {
            case 'c':
                clean = true;
                random = false;
                break;
            case 'b':
                bufsize = atoi(optarg);
                break;
            case 'l':
                loops = atoi(optarg);
                break;
            case 'r':
                clean = false;
                random = true;
                break;
            default:
                errx(EXIT_FAILURE, "Usage: %s [-c|-r] [-l LOOPS] [-b BUFFER]", argv[0]);
        }
    }

    int pfd[2];

    if (pipe(pfd) != 0) {
        err(EXIT_FAILURE, "pipe");
    }

    pid_t child = fork();
    if (child < 0) {
        err(EXIT_FAILURE, "fork");
    }

    if (child == 0) {
        close(pfd[0]);

        FILE *urandom = NULL;
        if (random) {
            urandom = fopen("/dev/urandom", "r");
            if (urandom == NULL) {
                err(EXIT_SUCCESS, "fopen(/dev/urandom)");
            }
        }

        for (int i = 0; i < loops; ++i) {
            uint8_t *buf;
            if (clean) {
                buf = (uint8_t *)calloc(bufsize, sizeof(uint8_t));
            }
            else {
                buf = (uint8_t *)malloc(bufsize * sizeof(uint8_t));
            }

            if (buf == NULL) {
                err(EXIT_SUCCESS, "malloc");
            }

            if (urandom) {
                const size_t r = fread(buf, sizeof(uint8_t), bufsize, urandom);
                if ((int)r < bufsize) {
                    err(EXIT_SUCCESS, "fread");
                }
            }

            fprintf(stdout, "%3.3f%%\r", ((i + 1.0)/(loops)) * 100);
            fflush(stdout);
        }

        if (urandom) {
            fclose(urandom);
            urandom = NULL;
        }

        fprintf(stdout, "Allocated %ld Bytes. Aborting ...\n", bufsize * loops);
        fflush(stdout);

        close(pfd[1]);

        abort();

        /* Dead code! */
        exit(EXIT_SUCCESS);
    }

    close(pfd[1]);

    int r, dummy, status;
    struct timespec beg, end;
    while ((r = read(pfd[0], &dummy, 1)) < 0) {
        if (errno != EAGAIN) {
            err(EXIT_FAILURE, "read");
        }
    }

    if (r != 0) {
        errx(EXIT_FAILURE, "The parent didn't get EOF");
    }

    r = clock_gettime(CLOCK_MONOTONIC_RAW, &beg);
    if (r != 0) {
        err(EXIT_FAILURE, "Begin: clock_gettime");
    }

    while ((r = waitpid(child, &status, 0)) < 0) {
        if (errno != EINTR) {
            err(EXIT_FAILURE, "waitpid");
        }
    }

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

    printf("Dumping core took: %lds %ldns\n", s, ns);

    close(pfd[0]);
    return EXIT_SUCCESS;
}
