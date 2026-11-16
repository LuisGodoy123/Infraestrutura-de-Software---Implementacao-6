#include <stdio.h>
#include <stdlib.h>
#include "banker.h"

int iniciar_estado(Estado *estado, int nc, int nr, const int *recursos, const int *maximo)
{
    int i;

    estado->n_clientes = nc;
    estado->n_recursos = nr;
    estado->disponivel = malloc(nr * sizeof(int));
    estado->maximo = malloc(nc * nr * sizeof(int));
    estado->alocado = calloc(nc * nr, sizeof(int));
    estado->necessidade = malloc(nc * nr * sizeof(int));
    if (estado->disponivel == NULL ||
        (nc > 0 && (estado->maximo == NULL || estado->alocado == NULL || estado->necessidade == NULL))) {
        fprintf(stderr, "Erro: sem memoria\n");
        free(estado->disponivel);
        free(estado->maximo);
        free(estado->alocado);
        free(estado->necessidade);
        return -1;
    }
    for (i = 0; i < nr; i++)
        estado->disponivel[i] = recursos[i];
    for (i = 0; i < nc * nr; i++) {
        estado->maximo[i] = maximo[i];
        estado->necessidade[i] = maximo[i];
    }
    return 0;
}

void liberar_estado(Estado *estado)
{
    free(estado->disponivel);
    free(estado->maximo);
    free(estado->alocado);
    free(estado->necessidade);
}

int estado_seguro(const Estado *estado)
{
    int nc = estado->n_clientes;
    int nr = estado->n_recursos;
    int *trabalho;
    int *terminou;
    int achou, ok;
    int i, j;

    if (nc == 0)
        return 1;
    trabalho = malloc(nr * sizeof(int));
    terminou = calloc(nc, sizeof(int));
    if (trabalho == NULL || terminou == NULL) {
        fprintf(stderr, "Erro: sem memoria\n");
        free(trabalho);
        free(terminou);
        return -1;
    }
    for (j = 0; j < nr; j++)
        trabalho[j] = estado->disponivel[j];
    do {
        achou = 0;
        for (i = 0; i < nc; i++) {
            if (terminou[i])
                continue;
            ok = 1;
            for (j = 0; j < nr; j++) {
                if (estado->necessidade[i * nr + j] > trabalho[j]) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                for (j = 0; j < nr; j++)
                    trabalho[j] += estado->alocado[i * nr + j];
                terminou[i] = 1;
                achou = 1;
            }
        }
    } while (achou);
    ok = 1;
    for (i = 0; i < nc; i++) {
        if (!terminou[i])
            ok = 0;
    }
    free(trabalho);
    free(terminou);
    return ok;
}

int pedir_recursos(Estado *estado, int cliente, const int *pedido)
{
    int nr = estado->n_recursos;
    int *necessidade = &estado->necessidade[cliente * nr];
    int *alocado = &estado->alocado[cliente * nr];
    int j, r;

    for (j = 0; j < nr; j++) {
        if (pedido[j] > necessidade[j])
            return PED_EXCEDE_MAXIMO;
    }
    for (j = 0; j < nr; j++) {
        if (pedido[j] > estado->disponivel[j])
            return PED_SEM_RECURSOS;
    }
    for (j = 0; j < nr; j++) {
        estado->disponivel[j] -= pedido[j];
        alocado[j] += pedido[j];
        necessidade[j] -= pedido[j];
    }
    r = estado_seguro(estado);
    if (r == 1)
        return PED_OK;
    for (j = 0; j < nr; j++) {
        estado->disponivel[j] += pedido[j];
        alocado[j] -= pedido[j];
        necessidade[j] += pedido[j];
    }
    if (r == 0)
        return PED_INSEGURO;
    return PED_ERRO;
}

int liberar_recursos(Estado *estado, int cliente, const int *devolucao)
{
    int nr = estado->n_recursos;
    int *necessidade = &estado->necessidade[cliente * nr];
    int *alocado = &estado->alocado[cliente * nr];
    int j;

    for (j = 0; j < nr; j++) {
        if (devolucao[j] > alocado[j])
            return -1;
    }
    for (j = 0; j < nr; j++) {
        estado->disponivel[j] += devolucao[j];
        alocado[j] -= devolucao[j];
        necessidade[j] += devolucao[j];
    }
    return 0;
}
