#include <stdio.h>
#include <stdlib.h>
#include "banker.h"

int init_state(State *s, int n, int m, const int *resources, const int *max)
{
    int i;

    s->n = n;
    s->m = m;
    s->available = malloc(m * sizeof(int));
    s->max = malloc(n * m * sizeof(int));
    s->alloc = calloc(n * m, sizeof(int));
    s->need = malloc(n * m * sizeof(int));
    if (s->available == NULL || (n > 0 && (s->max == NULL || s->alloc == NULL || s->need == NULL))) {
        fprintf(stderr, "Erro: sem memoria\n");
        free(s->available);
        free(s->max);
        free(s->alloc);
        free(s->need);
        return -1;
    }
    for (i = 0; i < m; i++)
        s->available[i] = resources[i];
    for (i = 0; i < n * m; i++) {
        s->max[i] = max[i];
        s->need[i] = max[i];
    }
    return 0;
}

int is_safe(const State *s)
{
    int *work;
    int *finish;
    int found, ok;
    int i, j;

    if (s->n == 0)
        return 1;
    work = malloc(s->m * sizeof(int));
    finish = calloc(s->n, sizeof(int));
    if (work == NULL || finish == NULL) {
        fprintf(stderr, "Erro: sem memoria\n");
        free(work);
        free(finish);
        return -1;
    }
    for (j = 0; j < s->m; j++)
        work[j] = s->available[j];
    do {
        found = 0;
        for (i = 0; i < s->n; i++) {
            if (finish[i])
                continue;
            ok = 1;
            for (j = 0; j < s->m; j++) {
                if (s->need[i * s->m + j] > work[j]) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                for (j = 0; j < s->m; j++)
                    work[j] += s->alloc[i * s->m + j];
                finish[i] = 1;
                found = 1;
            }
        }
    } while (found);
    ok = 1;
    for (i = 0; i < s->n; i++) {
        if (!finish[i])
            ok = 0;
    }
    free(work);
    free(finish);
    return ok;
}

int request_resources(State *s, int customer, const int *request)
{
    int m = s->m;
    int *need = &s->need[customer * m];
    int *alloc = &s->alloc[customer * m];
    int j, r;

    for (j = 0; j < m; j++) {
        if (request[j] > need[j])
            return REQ_EXCEEDS_MAX;
    }
    for (j = 0; j < m; j++) {
        if (request[j] > s->available[j])
            return REQ_NOT_ENOUGH;
    }
    for (j = 0; j < m; j++) {
        s->available[j] -= request[j];
        alloc[j] += request[j];
        need[j] -= request[j];
    }
    r = is_safe(s);
    if (r == 1)
        return REQ_OK;
    for (j = 0; j < m; j++) {
        s->available[j] += request[j];
        alloc[j] -= request[j];
        need[j] += request[j];
    }
    if (r == 0)
        return REQ_UNSAFE;
    return REQ_ERROR;
}

int release_resources(State *s, int customer, const int *release)
{
    int m = s->m;
    int *need = &s->need[customer * m];
    int *alloc = &s->alloc[customer * m];
    int j;

    for (j = 0; j < m; j++) {
        if (release[j] > alloc[j])
            return -1;
    }
    for (j = 0; j < m; j++) {
        s->available[j] += release[j];
        alloc[j] -= release[j];
        need[j] += release[j];
    }
    return 0;
}

void free_state(State *s)
{
    free(s->available);
    free(s->max);
    free(s->alloc);
    free(s->need);
}
