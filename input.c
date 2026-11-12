#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "banker.h"

int parse_int(const char *s, int *out)
{
    char *end;
    long v;

    if (s[0] < '0' || s[0] > '9')
        return -1;
    errno = 0;
    v = strtol(s, &end, 10);
    if (errno == ERANGE || *end != '\0' || v > INT_MAX)
        return -1;
    *out = v;
    return 0;
}

int read_args(int argc, char *argv[], int **resources)
{
    int m = argc - 1;
    int *r;
    int i;

    if (m < 1) {
        fprintf(stderr, "Erro: uso: ./banker <recurso1> <recurso2> ...\n");
        return -1;
    }
    r = malloc(m * sizeof(int));
    if (r == NULL) {
        fprintf(stderr, "Erro: sem memoria\n");
        return -1;
    }
    for (i = 0; i < m; i++) {
        if (parse_int(argv[i + 1], &r[i]) != 0) {
            fprintf(stderr, "Erro: argumento invalido: %s\n", argv[i + 1]);
            free(r);
            return -1;
        }
    }
    *resources = r;
    return m;
}

static int parse_row(char *line, int m, int *row)
{
    char *p = line;
    char *comma;
    int j;

    for (j = 0; j < m; j++) {
        comma = strchr(p, ',');
        if (comma != NULL)
            *comma = '\0';
        if (parse_int(p, &row[j]) != 0)
            return -1;
        if (comma == NULL)
            return j == m - 1 ? 0 : -1;
        p = comma + 1;
    }
    return -1;
}

int read_customers(const char *path, int m, const int *resources, int **max)
{
    FILE *f;
    char line[4096];
    int *data = NULL;
    int *aux;
    int n = 0;
    int lineno = 0;
    int blank = 0;
    int erro = 0;
    int len, j;

    f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", path);
        return -1;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        lineno++;
        len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            len--;
            line[len] = '\0';
        }
        if (len == 0) {
            blank = 1;
            continue;
        }
        if (blank) {
            fprintf(stderr, "Erro: %s: linha em branco no meio do arquivo\n", path);
            erro = 1;
            break;
        }
        aux = realloc(data, (n + 1) * m * sizeof(int));
        if (aux == NULL) {
            fprintf(stderr, "Erro: sem memoria\n");
            erro = 1;
            break;
        }
        data = aux;
        if (parse_row(line, m, &data[n * m]) != 0) {
            fprintf(stderr, "Erro: %s linha %d: formato invalido\n", path, lineno);
            erro = 1;
            break;
        }
        for (j = 0; j < m; j++) {
            if (data[n * m + j] > resources[j]) {
                fprintf(stderr, "Erro: %s linha %d: maximo maior que o total do recurso %d\n", path, lineno, j);
                erro = 1;
            }
        }
        if (erro)
            break;
        n++;
    }
    fclose(f);
    if (!erro && n == 0) {
        fprintf(stderr, "Erro: %s nao tem clientes\n", path);
        erro = 1;
    }
    if (erro) {
        free(data);
        return -1;
    }
    *max = data;
    return n;
}
