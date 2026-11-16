#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "banker.h"

int converter_inteiro(const char *texto, int *saida)
{
    char *fim;
    long v;

    if (texto[0] < '0' || texto[0] > '9')
        return -1;
    errno = 0;
    v = strtol(texto, &fim, 10);
    if (errno == ERANGE || *fim != '\0' || v > INT_MAX)
        return -1;
    *saida = v;
    return 0;
}

int ler_argumentos(int argc, char *argv[], int **recursos)
{
    int nr = argc - 1;
    int *r;
    int i;

    if (nr < 1) {
        fprintf(stderr, "Erro: uso: ./banker <recurso1> <recurso2> ...\n");
        return -1;
    }
    r = malloc(nr * sizeof(int));
    if (r == NULL) {
        fprintf(stderr, "Erro: sem memoria\n");
        return -1;
    }
    for (i = 0; i < nr; i++) {
        if (converter_inteiro(argv[i + 1], &r[i]) != 0) {
            fprintf(stderr, "Erro: argumento invalido: %s\n", argv[i + 1]);
            free(r);
            return -1;
        }
    }
    *recursos = r;
    return nr;
}

static int interpretar_linha_cliente(char *linha, int nr, int *vetor)
{
    char *p = linha;
    char *virgula;
    int j;

    for (j = 0; j < nr; j++) {
        virgula = strchr(p, ',');
        if (virgula != NULL)
            *virgula = '\0';
        if (converter_inteiro(p, &vetor[j]) != 0)
            return -1;
        if (virgula == NULL)
            return j == nr - 1 ? 0 : -1;
        p = virgula + 1;
    }
    return -1;
}

int ler_clientes(const char *caminho, int nr, int **maximo)
{
    FILE *f;
    char linha[4096];
    int *dados = NULL;
    int *aux;
    int n = 0;
    int num_linha = 0;
    int em_branco = 0;
    int erro = 0;
    int tam;

    f = fopen(caminho, "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", caminho);
        return -1;
    }
    while (fgets(linha, sizeof(linha), f) != NULL) {
        num_linha++;
        tam = strlen(linha);
        while (tam > 0 && (linha[tam - 1] == '\n' || linha[tam - 1] == '\r')) {
            tam--;
            linha[tam] = '\0';
        }
        if (tam == 0) {
            em_branco = 1;
            continue;
        }
        if (em_branco) {
            fprintf(stderr, "Erro: %s linha %d: formato invalido\n", caminho, num_linha);
            erro = 1;
            break;
        }
        aux = realloc(dados, (n + 1) * nr * sizeof(int));
        if (aux == NULL) {
            fprintf(stderr, "Erro: sem memoria\n");
            erro = 1;
            break;
        }
        dados = aux;
        if (interpretar_linha_cliente(linha, nr, &dados[n * nr]) != 0) {
            fprintf(stderr, "Erro: %s linha %d: formato invalido\n", caminho, num_linha);
            erro = 1;
            break;
        }
        n++;
    }
    fclose(f);
    if (erro) {
        free(dados);
        return -1;
    }
    *maximo = dados;
    return n;
}

static int proximo_inteiro(char **p, int *saida)
{
    char *espaco;
    int r;

    if (*p == NULL)
        return -1;
    espaco = strchr(*p, ' ');
    if (espaco != NULL)
        *espaco = '\0';
    r = converter_inteiro(*p, saida);
    if (espaco == NULL)
        *p = NULL;
    else
        *p = espaco + 1;
    return r;
}

static int interpretar_comando(char *linha, int nc, int nr, Comando *c)
{
    char *p;
    char *espaco;
    int j;

    c->quantidade = NULL;
    c->cliente = 0;
    if (strcmp(linha, "*") == 0) {
        c->tipo = '*';
        return 0;
    }
    espaco = strchr(linha, ' ');
    if (espaco == NULL)
        return -1;
    *espaco = '\0';
    if (strcmp(linha, "RQ") == 0)
        c->tipo = 'Q';
    else if (strcmp(linha, "RL") == 0)
        c->tipo = 'L';
    else
        return -1;
    p = espaco + 1;
    if (proximo_inteiro(&p, &c->cliente) != 0 || c->cliente >= nc)
        return -1;
    c->quantidade = malloc(nr * sizeof(int));
    if (c->quantidade == NULL)
        return -2;
    for (j = 0; j < nr; j++) {
        if (proximo_inteiro(&p, &c->quantidade[j]) != 0)
            break;
    }
    if (j < nr || p != NULL) {
        free(c->quantidade);
        c->quantidade = NULL;
        return -1;
    }
    return 0;
}

int ler_comandos(const char *caminho, int nc, int nr, Comando **comandos)
{
    FILE *f;
    char linha[4096];
    Comando *lista = NULL;
    Comando *aux;
    int total = 0;
    int num_linha = 0;
    int em_branco = 0;
    int erro = 0;
    int tam, i, r;

    f = fopen(caminho, "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", caminho);
        return -1;
    }
    while (fgets(linha, sizeof(linha), f) != NULL) {
        num_linha++;
        tam = strlen(linha);
        while (tam > 0 && (linha[tam - 1] == '\n' || linha[tam - 1] == '\r')) {
            tam--;
            linha[tam] = '\0';
        }
        if (tam == 0) {
            em_branco = 1;
            continue;
        }
        if (em_branco) {
            fprintf(stderr, "Erro: %s linha %d: formato invalido\n", caminho, num_linha);
            erro = 1;
            break;
        }
        aux = realloc(lista, (total + 1) * sizeof(Comando));
        if (aux == NULL) {
            fprintf(stderr, "Erro: sem memoria\n");
            erro = 1;
            break;
        }
        lista = aux;
        r = interpretar_comando(linha, nc, nr, &lista[total]);
        if (r == -2) {
            fprintf(stderr, "Erro: sem memoria\n");
            erro = 1;
            break;
        }
        if (r != 0) {
            fprintf(stderr, "Erro: %s linha %d: comando invalido\n", caminho, num_linha);
            erro = 1;
            break;
        }
        total++;
    }
    fclose(f);
    if (erro) {
        for (i = 0; i < total; i++)
            free(lista[i].quantidade);
        free(lista);
        return -1;
    }
    *comandos = lista;
    return total;
}

void liberar_comandos(Comando *comandos, int total)
{
    int i;

    for (i = 0; i < total; i++)
        free(comandos[i].quantidade);
    free(comandos);
}
