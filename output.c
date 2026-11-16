#include <stdio.h>
#include "banker.h"

static void print_vec(FILE *out, const int *v, int m, int width)
{
    int j;
    int count = 0;

    for (j = 0; j < m; j++)
        count += fprintf(out, "%d ", v[j]);
    for (; count < width; count++)
        fputc(' ', out);
}

void print_state(FILE *out, const State *s)
{
    int i;

    fprintf(out, "%-10s|   %-12s|   %s\n", "MAXIMUM", "ALLOCATION", "NEED");
    for (i = 0; i < s->n; i++) {
        print_vec(out, &s->max[i * s->m], s->m, 10);
        fprintf(out, "|   ");
        print_vec(out, &s->alloc[i * s->m], s->m, 12);
        fprintf(out, "|   ");
        print_vec(out, &s->need[i * s->m], s->m, 0);
        fputc('\n', out);
    }
    fprintf(out, "AVAILABLE ");
    print_vec(out, s->available, s->m, 0);
    fputc('\n', out);
}
