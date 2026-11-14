#ifndef BANKER_H
#define BANKER_H

typedef struct {
    char type;
    int customer;
    int *amount;
} Command;

typedef struct {
    int n;
    int m;
    int *available;
    int *max;
    int *alloc;
    int *need;
} State;

int parse_int(const char *s, int *out);
int read_args(int argc, char *argv[], int **resources);
int read_customers(const char *path, int m, int **max);
int read_commands(const char *path, int n, int m, Command **cmds);
int is_safe(const State *s);

#endif
