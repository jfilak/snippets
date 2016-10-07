#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

int main(int argc, char *argv[])
{
    for (int i = -1; i > -4; --i) {
        printf("%-12d %-12u %0X\n", i, i, i);
    }

    for (int i = INT_MIN + 3; i > INT_MIN; --i) {
        printf("%-12d %-12u %0X\n", i, i, i);
    }

    return EXIT_SUCCESS;
}
