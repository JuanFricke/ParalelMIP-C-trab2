#ifndef HELPER_H
#define HELPER_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Conta o número de palavras em uma string, onde cada palavra é separada por '+'
 * Esta função pode ser executada em paralelo usando MPI
 * 
 * @param input_string String de entrada contendo palavras separadas por '+'
 * @param world_rank Rank do processo MPI atual
 * @param world_size Número total de processos MPI
 * @return Número total de palavras encontradas
 */
int count_words_parallel(const char *input_string, int world_rank, int world_size);

/**
 * Encontra o nome que se repete mais vezes em uma lista de nomes
 * Esta função pode ser executada em paralelo usando MPI
 * 
 * @param names Array de strings contendo os nomes
 * @param num_names Número de nomes no array
 * @param world_rank Rank do processo MPI atual
 * @param world_size Número total de processos MPI
 * @return String contendo o nome mais frequente (deve ser liberada com free())
 */
char* find_most_frequent_name_parallel(char **names, int num_names, int world_rank, int world_size);

/**
 * Função auxiliar para contar palavras em uma substring
 * 
 * @param str String para contar palavras
 * @param start Posição inicial da substring
 * @param end Posição final da substring
 * @return Número de palavras na substring
 */
int count_words_in_substring(const char *str, int start, int end);

/**
 * Função auxiliar para contar frequência de um nome em uma lista
 * 
 * @param names Array de nomes
 * @param num_names Número total de nomes
 * @param start_index Índice inicial para busca
 * @param end_index Índice final para busca
 * @param target_name Nome para contar
 * @return Número de ocorrências do nome
 */
int count_name_frequency(char **names, int num_names, int start_index, int end_index, const char *target_name);

#endif // HELPER_H
