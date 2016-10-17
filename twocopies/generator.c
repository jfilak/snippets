#define  _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <signal.h>

#define BLOCK_SIZE 4096

sig_atomic_t interupt;

enum data_type
{
    DATA_UNINIT,
    DATA_ZEROS,
    DATA_RANDOM,
};

void sigint(int signo)
{
    signo = signo;
    interupt = 1;
}

int main(int argc, char *argv[])
{
    long bytes = -1;
    enum data_type tp = DATA_UNINIT;
    int opt;
    while ((opt = getopt(argc, argv, "rzb:c:")) != -1) {
        switch (opt) {
            case 'z':
                tp = DATA_ZEROS;
                break;
            case 'b':
                bytes = atol(optarg);
                break;
            case 'r':
                tp = DATA_RANDOM;
                break;
            default:
                errx(EXIT_FAILURE, "Usage: %s [-z|-r] [-b BYTES]", argv[0]);
        }
    }


    uint8_t *buf;
    if (tp == DATA_ZEROS) {
        buf = (uint8_t *)calloc(BLOCK_SIZE, sizeof(uint8_t));
    }
    else {
        buf = (uint8_t *)malloc(BLOCK_SIZE * sizeof(uint8_t));
    }

    if (buf == NULL) {
        err(EXIT_SUCCESS, "malloc");
    }

    if (tp == DATA_RANDOM) {
        FILE *urandom = fopen("/dev/urandom", "r");
        if (urandom == NULL) {
            err(EXIT_FAILURE, "fopen(/dev/urandom)");
        }
        const size_t r = fread(buf, sizeof(uint8_t), BLOCK_SIZE, urandom);
        if ((int)r < BLOCK_SIZE) {
            err(EXIT_FAILURE, "fread");
        }
        fclose(urandom);
    }

    signal(SIGINT, sigint);

    size_t total = 0;
    while (!interupt && (bytes < 0 || total < (size_t)bytes)) {
        int w = BLOCK_SIZE;
        if (bytes > 0 && (total + w > (size_t)bytes)) {
            w = bytes - total;
        }
        int incr = w;
        while (w) {
            int r = write(STDOUT_FILENO, buf, w);
            if (r < 0) {
                if (errno == EINTR) {
                    continue;
                }

                break;
            }
            w -= r;
        }

        fsync(STDOUT_FILENO);

        total += incr - w;

        if (w) {
            break;
        }
    }

    fprintf(stderr, "Wrote: %zu Bytes\n", total);
    free(buf);
    return EXIT_SUCCESS;
}
