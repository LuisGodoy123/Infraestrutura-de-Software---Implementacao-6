#ifndef BANKER_H
#define BANKER_H

#include <stdio.h>

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

enum {
    REQ_OK,
    REQ_EXCEEDS_MAX,
    REQ_NOT_ENOUGH,
    REQ_UNSAFE,
    REQ_ERROR
};

int parse_int(const char *s, int *out);
int read_args(int argc, char *argv[], int **resources);
int read_customers(const char *path, int m, int **max);
int read_commands(const char *path, int n, int m, Command **cmds);
int init_state(State *s, int n, int m, const int *resources, const int *max);
int is_safe(const State *s);
int request_resources(State *s, int customer, const int *request);
int release_resources(State *s, int customer, const int *release);
void print_state(FILE *out, const State *s);

#endif
