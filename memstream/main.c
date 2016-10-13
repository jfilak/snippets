#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

enum {
    TEST_NO_FLAGS = 0x0,
    TEST_PUTS_EMPTY_STRING = 0x1,
    TEST_PUTS_FOO_STRING = 0x2,
};

void test(int flags)
{
    char *buffer = NULL;
    size_t wrote = 0;

    FILE *stream = open_memstream(&buffer, &wrote);
    if (stream == NULL) {
        err(EXIT_FAILURE, "open_memstream");
    }

    fprintf(stream, "%s", "Hello, world!");
    fprintf(stream, "%s", "Dlanod");

    if (fseek(stream, strlen("Hello, world!"), SEEK_SET) < 0) {
        err(EXIT_FAILURE, "fseek");
    }

    if (flags & TEST_PUTS_EMPTY_STRING) {
        fputs("", stream);
    }

    if (flags & TEST_PUTS_FOO_STRING) {
        fputs("foo", stream);
    }

    fflush(stream);

    printf("flushed - len = %2zd : '%s'\n", wrote, buffer);

    fclose(stream);

    printf("closed  - len = %2zd : '%s'\n", wrote, buffer);

    free(buffer);
}

int main(void)
{
    printf("Seek back:\n");
    test(TEST_NO_FLAGS);

    printf("Seek back and fputs() empty string:\n");
    test(TEST_PUTS_EMPTY_STRING);

    printf("Seek back and fputs() foo string:\n");
    test(TEST_PUTS_FOO_STRING);

    return EXIT_SUCCESS;
}
