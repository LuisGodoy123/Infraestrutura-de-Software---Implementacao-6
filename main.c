#include <stdio.h>
#include <stdlib.h>
#include "banker.h"

int main(int argc, char *argv[])
{
    int *resources;
    int *max;
    Command *cmds;
    State s;
    FILE *out;
    int m, n, count, i, r;
    int erro = 0;

    m = read_args(argc, argv, &resources);
    if (m < 0)
        return 1;
    n = read_customers("customer.txt", m, &max);
    if (n < 0) {
        free(resources);
        return 1;
    }
    count = read_commands("commands.txt", n, m, &cmds);
    if (count < 0) {
        free(resources);
        free(max);
        return 1;
    }
    r = init_state(&s, n, m, resources, max);
    free(resources);
    free(max);
    if (r != 0) {
        free_commands(cmds, count);
        return 1;
    }
    out = fopen("result.txt", "w");
    if (out == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar result.txt\n");
        free_state(&s);
        free_commands(cmds, count);
        return 1;
    }
    for (i = 0; i < count && !erro; i++) {
        if (cmds[i].type == '*') {
            print_state(out, &s);
        } else if (cmds[i].type == 'L') {
            if (release_resources(&s, cmds[i].customer, cmds[i].amount) != 0) {
                fprintf(stderr, "Erro: comando %d: liberacao maior que o alocado\n", i + 1);
                erro = 1;
            } else {
                print_release(out, cmds[i].customer, cmds[i].amount, m);
            }
        } else {
            r = request_resources(&s, cmds[i].customer, cmds[i].amount);
            if (r == REQ_OK)
                print_allocate(out, cmds[i].customer, cmds[i].amount, m);
            else if (r == REQ_EXCEEDS_MAX)
                print_exceeds_max(out, cmds[i].customer, cmds[i].amount, m);
            else if (r == REQ_NOT_ENOUGH)
                print_not_enough(out, s.available, cmds[i].customer, cmds[i].amount, m);
            else if (r == REQ_UNSAFE)
                print_unsafe(out, cmds[i].customer, cmds[i].amount, m);
            else
                erro = 1;
        }
    }
    fclose(out);
    if (erro)
        remove("result.txt");
    free_state(&s);
    free_commands(cmds, count);
    return erro;
}
