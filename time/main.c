#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void print_time(const char *type, struct tm *ptm)
{
    fprintf(stdout, "%-9s %d-%d-%d %d-%d-%d\n", type, ptm->tm_year, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    return;
}

void print_env(void)
{
    char *const lc_time = getenv("LC_TIME");
    char *const tz = getenv("TZ");
    printf("LC_TIME=%6s TZ=%6s\n", lc_time, tz);
}

#define make_print_time(fn, arg) \
    print_time(#fn, fn(arg));

int main(int argc, char *argv[])
{
    time_t t = 0;

    print_env();
    make_print_time(localtime, &t);
    make_print_time(gmtime, &t);

    setenv("LC_TIME", "", 1);
    print_env();
    make_print_time(localtime, &t);
    make_print_time(gmtime, &t);

    setenv("TZ", "", 1);
    print_env();
    make_print_time(localtime, &t);
    make_print_time(gmtime, &t);

    return 0;
}
