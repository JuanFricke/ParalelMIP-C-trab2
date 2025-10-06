# Helper Functions para MPI

Este diretório contém funções auxiliares que podem ser executadas em paralelo usando MPI (Message Passing Interface).

## Arquivos

- `helper.h` - Cabeçalho com declarações das funções
- `helper.c` - Implementação das funções MPI
- `example_helper.c` - Exemplo de uso das funções
- `Makefile` - Atualizado para suportar compilação MPI

## Funções Disponíveis

### 1. `count_words_parallel()`

Conta o número de palavras em uma string, onde cada palavra é separada por '+'.

**Parâmetros:**
- `input_string`: String de entrada contendo palavras separadas por '+'
- `world_rank`: Rank do processo MPI atual
- `world_size`: Número total de processos MPI

**Retorno:**
- Número total de palavras encontradas

**Exemplo:**
```c
const char *test = "palavra1+palavra2+palavra3+palavra4";
int count = count_words_parallel(test, world_rank, world_size);
// Retorna: 4
```

### 2. `find_most_frequent_name_parallel()`

Encontra o nome que se repete mais vezes em uma lista de nomes.

**Parâmetros:**
- `names`: Array de strings contendo os nomes
- `num_names`: Número de nomes no array
- `world_rank`: Rank do processo MPI atual
- `world_size`: Número total de processos MPI

**Retorno:**
- String contendo o nome mais frequente (deve ser liberada com `free()`)

**Exemplo:**
```c
char *names[] = {"João", "Maria", "Pedro", "João", "Maria", "João"};
char *most_frequent = find_most_frequent_name_parallel(names, 6, world_rank, world_size);
// Retorna: "João" (aparece 3 vezes)
free(most_frequent);
```

## Como Compilar e Executar

### Pré-requisitos

Instale as dependências MPI:

**Ubuntu/Debian:**
```bash
make install-deps
```

**Fedora/RHEL:**
```bash
make install-deps-fedora
```

### Compilação

```bash
# Compilar a biblioteca helper
make libhelper.a

# Compilar o exemplo
make example_helper
```

### Execução

```bash
# Executar com 4 processos MPI
make run-helper

# Ou executar manualmente
mpirun -np 4 ./example_helper
```

## Algoritmo de Paralelização

### Contagem de Palavras

1. **Divisão de Dados**: A string é dividida entre os processos MPI
2. **Processamento Local**: Cada processo conta palavras em sua parte da string
3. **Sincronização**: Os resultados são somados usando `MPI_Reduce`
4. **Broadcast**: O resultado final é enviado para todos os processos

### Busca do Nome Mais Frequente

1. **Divisão de Dados**: A lista de nomes é dividida entre os processos
2. **Contagem Local**: Cada processo conta a frequência dos nomes em sua parte
3. **Redução Global**: O máximo global é encontrado usando `MPI_Allreduce`
4. **Seleção**: O nome mais frequente é selecionado entre os candidatos

## Estrutura do Código

```
helper.h          # Declarações das funções
helper.c          # Implementação das funções MPI
example_helper.c  # Exemplo de uso
Makefile          # Regras de compilação
```

## Notas Importantes

- As funções assumem que MPI já foi inicializado (`MPI_Init()`)
- O processo com rank 0 geralmente é responsável por imprimir resultados
- A memória alocada dinamicamente deve ser liberada pelo usuário
- As funções são thread-safe quando usadas corretamente com MPI

## Exemplo de Saída

```
=== Exemplo de Uso das Funções Helper ===
Executando com 4 processos MPI

1. Contando palavras separadas por '+'
=====================================
Processo 0: Contou 2 palavras na substring [0:20]
Processo 1: Contou 2 palavras na substring [20:40]
Processo 2: Contou 2 palavras na substring [40:60]
Processo 3: Contou 2 palavras na substring [60:80]
String: palavra1+palavra2+palavra3+palavra4+palavra5+palavra6+palavra7+palavra8
Total de palavras encontradas: 8

2. Encontrando nome mais frequente
==================================
Lista de nomes: João Maria Pedro Ana João Carlos Maria João Ana Pedro Maria João Carlos Ana João
Nome mais frequente: João
```
