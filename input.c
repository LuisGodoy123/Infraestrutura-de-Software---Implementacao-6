#include <stdio.h>
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

int read_args(int argc, char *argv[], int **resources)
{
    int m = argc - 1;
    int i;
    int *r;

    if (m < 1) {
        fprintf(stderr, "Erro: uso: ./banker <instancias> [<instancias> ...]\n");
        return -1;
    }
    r = malloc(m * sizeof(int));
    if (r == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente\n");
        return -1;
    }
    for (i = 0; i < m; i++) {
        if (parse_int(argv[i + 1], &r[i]) != 0) {
            fprintf(stderr, "Erro: argumento invalido '%s' (esperado inteiro nao negativo)\n", argv[i + 1]);
            free(r);
            return -1;
        }
    }
    *resources = r;
    return m;
}
