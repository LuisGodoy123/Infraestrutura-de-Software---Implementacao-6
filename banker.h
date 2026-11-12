#ifndef BANKER_H
#define BANKER_H

int parse_int(const char *s, int *out);
int read_args(int argc, char *argv[], int **resources);

#endif
