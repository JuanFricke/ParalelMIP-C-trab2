#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "helper.h"

int main(int argc, char *argv[]) {
    int world_rank, world_size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    if (world_rank == 0) {
        printf("=== Exemplo de Uso das Funções Helper ===\n");
        printf("Executando com %d processos MPI\n\n", world_size);
    }
    
    // Exemplo 1: Contar palavras separadas por '+'
    if (world_rank == 0) {
        printf("1. Contando palavras separadas por '+'\n");
        printf("=====================================\n");
    }
    
    const char *test_string = "palavra1+palavra2+palavra3+palavra4+palavra5+palavra6+palavra7+palavra8";
    int word_count = count_words_parallel(test_string, world_rank, world_size);
    
    if (world_rank == 0) {
        printf("String: %s\n", test_string);
        printf("Total de palavras encontradas: %d\n\n", word_count);
    }
    
    // Exemplo 2: Encontrar nome mais frequente
    if (world_rank == 0) {
        printf("2. Encontrando nome mais frequente\n");
        printf("==================================\n");
    }
    
    // Criar lista de nomes para teste
    char *names[] = {
        "João", "Maria", "Pedro", "Ana", "João",
        "Carlos", "Maria", "João", "Ana", "Pedro",
        "Maria", "João", "Carlos", "Ana", "João"
    };
    int num_names = 15;
    
    if (world_rank == 0) {
        printf("Lista de nomes: ");
        for (int i = 0; i < num_names; i++) {
            printf("%s ", names[i]);
        }
        printf("\n");
    }
    
    char *most_frequent = find_most_frequent_name_parallel(names, num_names, world_rank, world_size);
    
    if (world_rank == 0 && most_frequent != NULL) {
        printf("Nome mais frequente: %s\n", most_frequent);
        free(most_frequent);
    }
    
    MPI_Finalize();
    return 0;
}
