#include <stdio.h>
#include "banker.h"

static void imprimir_vetor(FILE *saida, const int *v, int n, int largura)
{
    int j;
    int escritos = 0;

    for (j = 0; j < n; j++)
        escritos += fprintf(saida, "%d ", v[j]);
    for (; escritos < largura; escritos++)
        fputc(' ', saida);
}

void imprimir_estado(FILE *saida, const Estado *estado)
{
    int nr = estado->n_recursos;
    int i;

    fprintf(saida, "%-10s|   %-12s|   %s\n", "MAXIMUM", "ALLOCATION", "NEED");
    for (i = 0; i < estado->n_clientes; i++) {
        imprimir_vetor(saida, &estado->maximo[i * nr], nr, 10);
        fprintf(saida, "|   ");
        imprimir_vetor(saida, &estado->alocado[i * nr], nr, 12);
        fprintf(saida, "|   ");
        imprimir_vetor(saida, &estado->necessidade[i * nr], nr, 0);
        fputc('\n', saida);
    }
    fprintf(saida, "AVAILABLE ");
    imprimir_vetor(saida, estado->disponivel, nr, 0);
    fputc('\n', saida);
}

static void imprimir_lista(FILE *saida, const int *v, int n)
{
    int j;

    for (j = 0; j < n; j++) {
        if (j > 0)
            fputc(' ', saida);
        fprintf(saida, "%d", v[j]);
    }
}

void imprimir_alocacao(FILE *saida, int cliente, const int *quantidade, int nr)
{
    fprintf(saida, "Allocate to customer %d the resources ", cliente);
    imprimir_lista(saida, quantidade, nr);
    fputc('\n', saida);
}

void imprimir_liberacao(FILE *saida, int cliente, const int *quantidade, int nr)
{
    fprintf(saida, "Release from customer %d the resources ", cliente);
    imprimir_lista(saida, quantidade, nr);
    fputc('\n', saida);
}

void imprimir_sem_recursos(FILE *saida, const int *disponivel, int cliente, const int *pedido, int nr)
{
    fprintf(saida, "The resources ");
    imprimir_lista(saida, disponivel, nr);
    fprintf(saida, " are not enough to customer %d request ", cliente);
    imprimir_lista(saida, pedido, nr);
    fprintf(saida, " \n");
}

void imprimir_inseguro(FILE *saida, int cliente, const int *pedido, int nr)
{
    fprintf(saida, "The customer %d request ", cliente);
    imprimir_lista(saida, pedido, nr);
    fprintf(saida, " was denied because result in an unsafe state\n");
}

void imprimir_excede_maximo(FILE *saida, int cliente, const int *pedido, int nr)
{
    fprintf(saida, "The customer %d request ", cliente);
    imprimir_lista(saida, pedido, nr);
    fprintf(saida, " was denied because exceed its maximum allocation\n");
}
