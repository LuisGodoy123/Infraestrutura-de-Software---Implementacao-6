#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "banker.h"

int parse_int(const char *s, int *out)
{
    char *end;
    long v;

    if (!isdigit((unsigned char)s[0]))
        return -1;
    errno = 0;
    v = strtol(s, &end, 10);
    if (errno == ERANGE || *end != '\0' || v > INT_MAX)
        return -1;
    *out = (int)v;
    return 0;
}
