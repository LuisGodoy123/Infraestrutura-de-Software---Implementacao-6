#include <stdio.h>
#include <stdlib.h>
#include "banker.h"

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
