#ifndef BANKER_H
#define BANKER_H

#include <stdio.h>

typedef struct {
    int n_clientes;
    int n_recursos;
    int *disponivel;
    int *maximo;
    int *alocado;
    int *necessidade;
} Estado;

typedef struct {
    char tipo;
    int cliente;
    int *quantidade;
} Comando;

enum {
    PED_OK,
    PED_EXCEDE_MAXIMO,
    PED_SEM_RECURSOS,
    PED_INSEGURO,
    PED_ERRO
};

int converter_inteiro(const char *texto, int *saida);
int ler_argumentos(int argc, char *argv[], int **recursos);
int ler_clientes(const char *caminho, int nr, int **maximo);
int ler_comandos(const char *caminho, int nc, int nr, Comando **comandos);
void liberar_comandos(Comando *comandos, int total);
int iniciar_estado(Estado *estado, int nc, int nr, const int *recursos, const int *maximo);
void liberar_estado(Estado *estado);
int estado_seguro(const Estado *estado);
int pedir_recursos(Estado *estado, int cliente, const int *pedido);
int liberar_recursos(Estado *estado, int cliente, const int *devolucao);
void imprimir_estado(FILE *saida, const Estado *estado);
void imprimir_alocacao(FILE *saida, int cliente, const int *quantidade, int nr);
void imprimir_liberacao(FILE *saida, int cliente, const int *quantidade, int nr);
void imprimir_sem_recursos(FILE *saida, const int *disponivel, int cliente, const int *pedido, int nr);
void imprimir_inseguro(FILE *saida, int cliente, const int *pedido, int nr);
void imprimir_excede_maximo(FILE *saida, int cliente, const int *pedido, int nr);

#endif
