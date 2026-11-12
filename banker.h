#ifndef BANKER_H
#define BANKER_H

int parse_int(const char *s, int *out);
int read_args(int argc, char *argv[], int **resources);
int read_customers(const char *path, int m, const int *resources, int **max);

#endif
