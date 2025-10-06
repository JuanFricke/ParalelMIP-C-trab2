#include "helper.h"

int count_words_parallel(const char *input_string, int world_rank, int world_size) {
    int string_length = strlen(input_string);
    int total_words = 0;
    
    if (world_rank == 0) {
        printf("Processo %d: Contando palavras na string: %s\n", world_rank, input_string);
    }
    
    // Dividir a string entre os processos
    int chunk_size = string_length / world_size;
    int start = world_rank * chunk_size;
    int end = (world_rank == world_size - 1) ? string_length : (world_rank + 1) * chunk_size;
    
    // Ajustar para não cortar palavras no meio
    if (world_rank > 0 && start < string_length && input_string[start] != '+') {
        // Encontrar o próximo '+'
        while (start < string_length && input_string[start] != '+') {
            start++;
        }
        if (start < string_length) start++; // Pular o '+'
    }
    
    if (world_rank < world_size - 1 && end < string_length && input_string[end] != '+') {
        // Encontrar o próximo '+'
        while (end < string_length && input_string[end] != '+') {
            end++;
        }
    }
    
    // Contar palavras na parte atribuída a este processo
    int local_words = count_words_in_substring(input_string, start, end);
    
    printf("Processo %d: Contou %d palavras na substring [%d:%d]\n", 
           world_rank, local_words, start, end);
    
    // Reduzir os resultados de todos os processos
    MPI_Reduce(&local_words, &total_words, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Broadcast o resultado para todos os processos
    MPI_Bcast(&total_words, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    return total_words;
}

char* find_most_frequent_name_parallel(char **names, int num_names, int world_rank, int world_size) {
    char *most_frequent = NULL;
    int max_count = 0;
    
    if (world_rank == 0) {
        printf("Processo %d: Procurando nome mais frequente em %d nomes\n", world_rank, num_names);
    }
    
    // Dividir a lista de nomes entre os processos
    int chunk_size = num_names / world_size;
    int start = world_rank * chunk_size;
    int end = (world_rank == world_size - 1) ? num_names : (world_rank + 1) * chunk_size;
    
    printf("Processo %d: Analisando nomes de %d a %d\n", world_rank, start, end - 1);
    
    // Para cada nome na parte atribuída a este processo
    for (int i = start; i < end; i++) {
        if (names[i] == NULL) continue;
        
        // Contar quantas vezes este nome aparece em toda a lista
        int count = count_name_frequency(names, num_names, 0, num_names, names[i]);
        
        printf("Processo %d: Nome '%s' aparece %d vezes\n", world_rank, names[i], count);
        
        // Atualizar o máximo local se necessário
        if (count > max_count) {
            max_count = count;
            if (most_frequent != NULL) {
                free(most_frequent);
            }
            most_frequent = strdup(names[i]);
        }
    }
    
    // Encontrar o máximo global entre todos os processos
    int global_max_count;
    MPI_Allreduce(&max_count, &global_max_count, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    
    // Se este processo tem o máximo global, enviar o nome para o processo 0
    char *global_most_frequent = NULL;
    int name_length = 0;
    
    if (max_count == global_max_count && most_frequent != NULL) {
        name_length = strlen(most_frequent) + 1;
    }
    
    // Encontrar o comprimento máximo do nome
    int max_name_length;
    MPI_Allreduce(&name_length, &max_name_length, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    
    // Alocar buffer para o nome
    char *name_buffer = NULL;
    if (max_name_length > 0) {
        name_buffer = (char*)malloc(max_name_length);
        memset(name_buffer, 0, max_name_length);
        
        if (max_count == global_max_count && most_frequent != NULL) {
            strcpy(name_buffer, most_frequent);
        }
    }
    
    // Encontrar o nome mais frequente globalmente
    // Usar uma abordagem simples: o processo com rank menor tem prioridade
    for (int rank = 0; rank < world_size; rank++) {
        char *temp_buffer = (char*)malloc(max_name_length);
        memset(temp_buffer, 0, max_name_length);
        
        if (rank == world_rank && name_buffer != NULL) {
            strcpy(temp_buffer, name_buffer);
        }
        
        MPI_Bcast(temp_buffer, max_name_length, MPI_CHAR, rank, MPI_COMM_WORLD);
        
        if (rank == 0 && temp_buffer[0] != '\0') {
            global_most_frequent = strdup(temp_buffer);
            break;
        }
        
        free(temp_buffer);
    }
    
    // Limpar memória local
    if (most_frequent != NULL) {
        free(most_frequent);
    }
    if (name_buffer != NULL) {
        free(name_buffer);
    }
    
    return global_most_frequent;
}

int count_words_in_substring(const char *str, int start, int end) {
    int count = 0;
    int in_word = 0;
    
    for (int i = start; i < end; i++) {
        if (str[i] == '+') {
            if (in_word) {
                count++;
                in_word = 0;
            }
        } else if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
            in_word = 1;
        }
    }
    
    // Contar a última palavra se a string não terminar com '+'
    if (in_word) {
        count++;
    }
    
    return count;
}

int count_name_frequency(char **names, int num_names, int start_index, int end_index, const char *target_name) {
    int count = 0;
    
    for (int i = start_index; i < end_index; i++) {
        if (names[i] != NULL && strcmp(names[i], target_name) == 0) {
            count++;
        }
    }
    
    return count;
}
