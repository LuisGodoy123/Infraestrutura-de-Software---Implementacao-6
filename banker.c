#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

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


static int converter_inteiro(const char *texto, int *saida)
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

static int ler_argumentos(int argc, char *argv[], int **recursos)
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

static int ler_clientes(const char *caminho, int nr, int **maximo)
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

static int ler_comandos(const char *caminho, int nc, int nr, Comando **comandos)
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

static void liberar_comandos(Comando *comandos, int total)
{
    int i;

    for (i = 0; i < total; i++)
        free(comandos[i].quantidade);
    free(comandos);
}

static int iniciar_estado(Estado *estado, int nc, int nr, const int *recursos, const int *maximo)
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

static void liberar_estado(Estado *estado)
{
    free(estado->disponivel);
    free(estado->maximo);
    free(estado->alocado);
    free(estado->necessidade);
}

static int estado_seguro(const Estado *estado)
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

static int pedir_recursos(Estado *estado, int cliente, const int *pedido)
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

static int liberar_recursos(Estado *estado, int cliente, const int *devolucao)
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

static void imprimir_vetor(FILE *saida, const int *v, int n, int largura)
{
    int j;
    int escritos = 0;

    for (j = 0; j < n; j++)
        escritos += fprintf(saida, "%d ", v[j]);
    for (; escritos < largura; escritos++)
        fputc(' ', saida);
}

static void imprimir_estado(FILE *saida, const Estado *estado)
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

static void imprimir_alocacao(FILE *saida, int cliente, const int *quantidade, int nr)
{
    fprintf(saida, "Allocate to customer %d the resources ", cliente);
    imprimir_lista(saida, quantidade, nr);
    fputc('\n', saida);
}

static void imprimir_liberacao(FILE *saida, int cliente, const int *quantidade, int nr)
{
    fprintf(saida, "Release from customer %d the resources ", cliente);
    imprimir_lista(saida, quantidade, nr);
    fputc('\n', saida);
}

static void imprimir_sem_recursos(FILE *saida, const int *disponivel, int cliente, const int *pedido, int nr)
{
    fprintf(saida, "The resources ");
    imprimir_lista(saida, disponivel, nr);
    fprintf(saida, " are not enough to customer %d request ", cliente);
    imprimir_lista(saida, pedido, nr);
    fprintf(saida, " \n");
}

static void imprimir_inseguro(FILE *saida, int cliente, const int *pedido, int nr)
{
    fprintf(saida, "The customer %d request ", cliente);
    imprimir_lista(saida, pedido, nr);
    fprintf(saida, " was denied because result in an unsafe state\n");
}

static void imprimir_excede_maximo(FILE *saida, int cliente, const int *pedido, int nr)
{
    fprintf(saida, "The customer %d request ", cliente);
    imprimir_lista(saida, pedido, nr);
    fprintf(saida, " was denied because exceed its maximum allocation\n");
}

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
