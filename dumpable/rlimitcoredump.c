#if 0
BINARY=${0/%.c/}
gcc -Wall -Wextra -std=c11 -pedantic -o $BINARY $0 || exit 1

ulimit -c 0

sudo rm -rf /va/tmp/1core*

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
    struct rlimit core;
    getrlimit(RLIMIT_CORE, &core);
    printf("%lld %lld", core.rlim_cur, core.rlim_max);
    abort();
}
