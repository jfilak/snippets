#if 0
BINARY=${0/%.c/}
gcc -Wall -Wextra -std=c11 -pedantic -o $BINARY $0 || exit 1

OLD_PATTERN=$(sysctl kernel.core_pattern -n)

sudo sysctl kernel.core_pattern="/var/tmp/1core"

$PWD/$BINARY

sudo sysctl kernel.core_pattern="|/bin/tee /var/tmp/1core.pipe"

$PWD/$BINARY

sudo sysctl kernel.core_pattern="$OLD_PATTERN"

ls /var/tmp/1core*
exit
#endif

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(void)
{
    setrlimit(RLIMIT_CORE, &((struct rlimit){ 1, 1 }));
    abort();
}
