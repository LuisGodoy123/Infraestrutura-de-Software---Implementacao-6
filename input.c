#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int read_customers(const char *path, int m, const int *resources, int **max)
{
    FILE *f;
    char line[4096];
    int *data = NULL;
    int *tmp;
    int n = 0;
    int lineno = 0;
    int blank = 0;
    size_t len;
    char *p, *comma;
    int j;

    f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", path);
        return -1;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        lineno++;
        len = strlen(line);
        if (len > 0 && line[len - 1] != '\n' && !feof(f)) {
            fprintf(stderr, "Erro: %s linha %d: linha longa demais\n", path, lineno);
            goto fail;
        }
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0) {
            blank = 1;
            continue;
        }
        if (blank) {
            fprintf(stderr, "Erro: %s: linha em branco no meio do arquivo\n", path);
            goto fail;
        }
        tmp = realloc(data, (n + 1) * m * sizeof(int));
        if (tmp == NULL) {
            fprintf(stderr, "Erro: memoria insuficiente\n");
            goto fail;
        }
        data = tmp;
        p = line;
        for (j = 0; j < m; j++) {
            comma = strchr(p, ',');
            if (j < m - 1) {
                if (comma == NULL) {
                    fprintf(stderr, "Erro: %s linha %d: campos a menos (esperado %d)\n", path, lineno, m);
                    goto fail;
                }
                *comma = '\0';
            } else if (comma != NULL) {
                fprintf(stderr, "Erro: %s linha %d: campos a mais (esperado %d)\n", path, lineno, m);
                goto fail;
            }
            if (parse_int(p, &data[n * m + j]) != 0) {
                fprintf(stderr, "Erro: %s linha %d: valor invalido '%s'\n", path, lineno, p);
                goto fail;
            }
            if (data[n * m + j] > resources[j]) {
                fprintf(stderr, "Erro: %s linha %d: maximo %d excede o total %d do recurso %d\n",
                        path, lineno, data[n * m + j], resources[j], j);
                goto fail;
            }
            if (comma != NULL)
                p = comma + 1;
        }
        n++;
    }
    if (n == 0) {
        fprintf(stderr, "Erro: %s nao tem nenhum cliente\n", path);
        goto fail;
    }
    fclose(f);
    *max = data;
    return n;

fail:
    free(data);
    fclose(f);
    return -1;
}
