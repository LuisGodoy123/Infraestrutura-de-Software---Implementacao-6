# Implementação 6 - Algoritmo de Banker

Login: `lagi`. Infraestrutura de Software, CESAR School.

Implementação em C do algoritmo de Banker (Silberschatz, *Operating System Concepts*, 10a edição). O número de clientes vem do `customer.txt` e o número de recursos vem dos argumentos da linha de comando.

## Arquivos

| Arquivo | O que faz |
|---|---|
| `main.c` | Orquestra: lê e valida tudo antes de executar, roda os comandos e escreve o `result.txt`. |
| `input.c` | Leitura e validação dos argumentos, do `customer.txt` e do `commands.txt`. |
| `banker.c` | Lógica do algoritmo: estado, verificação de segurança, pedido (`RQ`) e liberação (`RL`). |
| `output.c` | Escrita das linhas e da tabela do `result.txt`. |
| `banker.h` | Structs e protótipos. |
| `Makefile` | `make` gera `banker`; `make clean` remove objetos, executável e `result.txt`. |
| `evidencias.log` | Registro dos testes feitos durante o desenvolvimento. |

## Como compilar e executar

```
make
./banker 10 5 7
```

Cada argumento é o número de instâncias de um tipo de recurso. O programa lê `customer.txt` e `commands.txt` do diretório atual e escreve `result.txt` no mesmo diretório. Nada é impresso em stdout.

## Ambiente

Ubuntu 26.04 (WSL no Windows 11), gcc 15.2.0, GNU Make 4.4.1. Compilado com `-Wall -Wextra -std=c99`, sem warnings. Não usa `math.h` nem bibliotecas além da padrão do C.

## Erros

Em qualquer erro o programa escreve uma mensagem em stderr, termina com código 1 e **não cria `result.txt`**. Os arquivos são validados por inteiro antes de qualquer comando ser executado. São erros:
- número de argumentos diferente do número de colunas do `customer.txt` ou de valores nos comandos;
- argumento que não seja inteiro não negativo;
- `customer.txt` ou `commands.txt` inexistente, que não abre ou em formato diferente do enunciado.

## Formato da saída

Segue o `result.txt` de exemplo do professor (inclusive os espaços no fim de algumas linhas). Onde o PDF e o `result.txt` do professor divergem (`The resources 3 2 2` no PDF, `3 3 2` no arquivo), vale o arquivo.

## Pontos que o enunciado não define

- Um `RL` que libera mais do que o cliente tem alocado, ou um cliente que não existe no `customer.txt`: tratados como erro (o primeiro é detectado durante a execução, e nesse caso o `result.txt` já aberto é removido).
- Arquivos com CRLF e linhas em branco no fim são aceitos.
- `customer.txt` sem linhas é lido como 0 clientes; `commands.txt` vazio gera `result.txt` vazio.
