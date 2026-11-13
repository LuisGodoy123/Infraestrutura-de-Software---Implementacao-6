#ifndef BANKER_H
#define BANKER_H

typedef struct {
    char type;
    int customer;
    int *amount;
} Command;

int parse_int(const char *s, int *out);
int read_args(int argc, char *argv[], int **resources);
int read_customers(const char *path, int m, int **max);
int read_commands(const char *path, int n, int m, Command **cmds);

#endif
