#include <stdio.h>
#include <stdlib.h>
#include "banker.h"

int main(int argc, char *argv[])
{
    int *recursos;
    int *maximo;
    Comando *comandos;
    Estado estado;
    FILE *saida;
    int nr, nc, total, i, r;
    int erro = 0;

    nr = ler_argumentos(argc, argv, &recursos);
    if (nr < 0)
        return 1;
    nc = ler_clientes("customer.txt", nr, &maximo);
    if (nc < 0) {
        free(recursos);
        return 1;
    }
    total = ler_comandos("commands.txt", nc, nr, &comandos);
    if (total < 0) {
        free(recursos);
        free(maximo);
        return 1;
    }
    r = iniciar_estado(&estado, nc, nr, recursos, maximo);
    free(recursos);
    free(maximo);
    if (r != 0) {
        liberar_comandos(comandos, total);
        return 1;
    }
    saida = fopen("result.txt", "w");
    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar result.txt\n");
        liberar_estado(&estado);
        liberar_comandos(comandos, total);
        return 1;
    }
    for (i = 0; i < total && !erro; i++) {
        if (comandos[i].tipo == '*') {
            imprimir_estado(saida, &estado);
        } else if (comandos[i].tipo == 'L') {
            if (liberar_recursos(&estado, comandos[i].cliente, comandos[i].quantidade) != 0) {
                fprintf(stderr, "Erro: comando %d: liberacao maior que o alocado\n", i + 1);
                erro = 1;
            } else {
                imprimir_liberacao(saida, comandos[i].cliente, comandos[i].quantidade, nr);
            }
        } else {
            r = pedir_recursos(&estado, comandos[i].cliente, comandos[i].quantidade);
            if (r == PED_OK)
                imprimir_alocacao(saida, comandos[i].cliente, comandos[i].quantidade, nr);
            else if (r == PED_EXCEDE_MAXIMO)
                imprimir_excede_maximo(saida, comandos[i].cliente, comandos[i].quantidade, nr);
            else if (r == PED_SEM_RECURSOS)
                imprimir_sem_recursos(saida, estado.disponivel, comandos[i].cliente, comandos[i].quantidade, nr);
            else if (r == PED_INSEGURO)
                imprimir_inseguro(saida, comandos[i].cliente, comandos[i].quantidade, nr);
            else
                erro = 1;
        }
    }
    fclose(saida);
    if (erro)
        remove("result.txt");
    liberar_estado(&estado);
    liberar_comandos(comandos, total);
    return erro;
}
