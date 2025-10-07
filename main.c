// Inclusão das bibliotecas necessárias
#include <stdio.h>    // Para entrada e saída padrão
#include <stdlib.h>   // Para funções de alocação de memória
#include <string.h>   // Para manipulação de strings
#include <ctype.h>    // Para funções de caracteres (isalpha, etc.)
#include <mpi.h>      // Para programação paralela com MPI
#include "ollama_client.h"  // Para comunicação com o modelo de IA Ollama

// Definições de constantes para limites de tamanho
#define MAX_LINE_LENGTH 100000    // Tamanho máximo de uma linha do CSV
#define MAX_WORD_LENGTH 100      // Tamanho máximo de uma palavra
#define MAX_ARTIST_LENGTH 200    // Tamanho máximo do nome do artista
#define MAX_SONG_LENGTH 200      // Tamanho máximo do nome da música
#define MAX_TEXT_LENGTH 10000     // Tamanho máximo do texto da letra
#define MAX_WORDS 50000000          // Número máximo de palavras únicas
#define MAX_ARTISTS 5000         // Número máximo de artistas únicos
#define MAX_LLM_SONGS 200        // Número máximo de músicas para análise de sentimento
#define IO_BUFFER_SIZE 1024 * 1024  // Buffer de 1MB para otimização de I/O
#define LINES_PER_CHUNK 100      // Processar 100 linhas por vez

// Estrutura para armazenar contagem de palavras
typedef struct {
    char word[MAX_WORD_LENGTH];  // A palavra em si
    int count;                   // Quantas vezes a palavra aparece
} WordCount;

// Estrutura para armazenar contagem de artistas
typedef struct {
    char artist[MAX_ARTIST_LENGTH];  // Nome do artista
    int song_count;                  // Quantas músicas o artista tem
} ArtistCount;

// Estrutura para armazenar dados de uma música
typedef struct {
    char artist[MAX_ARTIST_LENGTH];  // Nome do artista
    char song[MAX_SONG_LENGTH];      // Nome da música
    char text[MAX_TEXT_LENGTH];      // Letra da música
} SongData;

// Protótipos das funções
int count_csv_lines(const char* filename);  // Conta o número de linhas no arquivo CSV
void read_file_chunk_optimized(const char* filename, int start_line, int num_lines, SongData* songs, int* actual_lines);  // Lê um pedaço do arquivo otimizado
void count_words_io_optimized(const char* filename, int total_songs, WordCount* word_counts, int* num_words, int world_rank, int world_size);  // Conta palavras de forma paralela
void count_artists_io_optimized(const char* filename, int total_songs, ArtistCount* artist_counts, int* num_artists, int world_rank, int world_size);  // Conta artistas de forma paralela
void classify_sentiments_io_optimized(const char* filename, int total_songs, int* sentiment_counts, int world_rank, int world_size);  // Classifica sentimentos usando IA
void print_results(WordCount* word_counts, int num_words, ArtistCount* artist_counts, int num_artists, int* sentiment_counts);  // Imprime os resultados finais
int compare_word_counts(const void* a, const void* b);  // Função de comparação para ordenar palavras por frequência
int compare_artist_counts(const void* a, const void* b);  // Função de comparação para ordenar artistas por número de músicas

int main(int argc, char* argv[]) {
    int world_rank, world_size;  // Variáveis para identificar o processo atual e total de processos
    
    // Inicializa o ambiente MPI (Message Passing Interface)
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);  // Obtém o ID do processo atual (0, 1, 2, ...)
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);  // Obtém o número total de processos
    
    // Apenas o processo 0 (mestre) imprime informações iniciais
    if (world_rank == 0) {
        printf("Programa de Análise de Música - Versão MPI\n");
        printf("==================================================\n");
        printf("Usando %d processos MPI\n", world_size);
        printf("Tamanho do buffer I/O: %d bytes\n", IO_BUFFER_SIZE);
        printf("Linhas por pedaço: %d\n", LINES_PER_CHUNK);
        printf("Máximo de músicas para LLM: %d (para economizar tempo e memória)\n\n", MAX_LLM_SONGS);
    }
    
    // Obtém o número total de músicas (apenas o processo 0)
    int total_songs = 0;
    if (world_rank == 0) {
        total_songs = count_csv_lines("test_music.csv");
        if (total_songs <= 0) {
            printf("Erro: Não foi possível ler o arquivo CSV ou o arquivo está vazio\n");
            MPI_Abort(MPI_COMM_WORLD, 1);  // Termina todos os processos se houver erro
        }
        printf("Encontradas %d músicas no arquivo CSV\n", total_songs);
    }
    
    // Transmite o número total de músicas para todos os processos
    MPI_Bcast(&total_songs, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Arrays para armazenar os resultados
    WordCount* word_counts = NULL;  // Array para contagem de palavras
    int num_words = 0;              // Número de palavras únicas encontradas
    ArtistCount* artist_counts = NULL;  // Array para contagem de artistas
    int num_artists = 0;            // Número de artistas únicos encontrados
    int sentiment_counts[3] = {0, 0, 0}; // [Positivo, Neutro, Negativo]
    
    // Apenas o processo 0 aloca memória para os resultados finais
    if (world_rank == 0) {
        word_counts = (WordCount*)malloc(MAX_WORDS * sizeof(WordCount));
        artist_counts = (ArtistCount*)malloc(MAX_ARTISTS * sizeof(ArtistCount));
    }
    
    // 1. Contagem de Palavras - Análise paralela
    if (world_rank == 0) {
        printf("\n1. Análise de Contagem de Palavras - \n");
        printf("=======================================================\n");
    }
    count_words_io_optimized("test_music.csv", total_songs, word_counts, &num_words, world_rank, world_size);
    
    // 2. Análise de Artistas - Contagem paralela
    if (world_rank == 0) {
        printf("\n2. Análise de Artistas - \n");
        printf("===============================================\n");
    }
    count_artists_io_optimized("test_music.csv", total_songs, artist_counts, &num_artists, world_rank, world_size);
    
    // 3. Classificação de Sentimento - Usando IA
    if (world_rank == 0) {
        printf("\n3. Classificação de Sentimento - \n");
        printf("========================================================\n");
    }
    classify_sentiments_io_optimized("test_music.csv", total_songs, sentiment_counts, world_rank, world_size);
    
    // Imprime os resultados (apenas o processo 0)
    if (world_rank == 0) {
        print_results(word_counts, num_words, artist_counts, num_artists, sentiment_counts);
    }
    
    // Limpeza da memória
    if (world_rank == 0) {
        free(word_counts);
        free(artist_counts);
    }
    
    // Finaliza o ambiente MPI
    MPI_Finalize();
    return 0;
}

// Função para contar o número de linhas no arquivo CSV
int count_csv_lines(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;  // Retorna erro se não conseguir abrir o arquivo
    
    // Otimiza I/O com buffer grande para melhor performance
    char* buffer = (char*)malloc(IO_BUFFER_SIZE);
    setvbuf(file, buffer, _IOFBF, IO_BUFFER_SIZE);
    
    char line[MAX_LINE_LENGTH];
    int count = 0;
    
    // Pula o cabeçalho do CSV (primeira linha)
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        free(buffer);
        return -1;
    }
    
    // Conta as linhas restantes (dados das músicas)
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    
    fclose(file);
    free(buffer);
    return count;
}

// Função para ler um pedaço do arquivo CSV de forma otimizada
void read_file_chunk_optimized(const char* filename, int start_line, int num_lines, SongData* songs, int* actual_lines) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        *actual_lines = 0;
        return;
    }
    
    // Otimiza I/O com buffer grande para melhor performance
    char* buffer = (char*)malloc(IO_BUFFER_SIZE);
    setvbuf(file, buffer, _IOFBF, IO_BUFFER_SIZE);
    
    char line[MAX_LINE_LENGTH];
    int current_line = 0;
    int count = 0;
    
    // Pula o cabeçalho do CSV
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        free(buffer);
        *actual_lines = 0;
        return;
    }
    
    // Pula até a linha de início desejada
    while (current_line < start_line && fgets(line, sizeof(line), file)) {
        current_line++;
    }
    
    // Lê o pedaço com parsing otimizado
    while (count < num_lines && fgets(line, sizeof(line), file)) {
        // Parsing rápido do CSV - encontra delimitadores manualmente
        char* artist_start = line;  // Início do nome do artista
        char* song_start = strchr(line, '|');  // Procura o primeiro separador
        if (!song_start) continue;  // Pula se não encontrar o separador
        *song_start++ = '\0';  // Termina a string do artista
        
        char* text_start = strchr(song_start, '|');  // Procura o segundo separador
        if (!text_start) continue;  // Pula se não encontrar o separador
        *text_start++ = '\0';  // Termina a string da música
        
        // Remove quebra de linha do texto
        char* newline = strchr(text_start, '\n');
        if (newline) *newline = '\0';
        
        // Copia para a estrutura com verificação de limites
        int artist_len = strlen(artist_start);
        int song_len = strlen(song_start);
        int text_len = strlen(text_start);
        
        // Calcula o tamanho a copiar respeitando os limites máximos
        int copy_artist_len = (artist_len < MAX_ARTIST_LENGTH - 1) ? artist_len : MAX_ARTIST_LENGTH - 1;
        int copy_song_len = (song_len < MAX_SONG_LENGTH - 1) ? song_len : MAX_SONG_LENGTH - 1;
        int copy_text_len = (text_len < MAX_TEXT_LENGTH - 1) ? text_len : MAX_TEXT_LENGTH - 1;
        
        // Copia os dados para a estrutura
        strncpy(songs[count].artist, artist_start, copy_artist_len);
        songs[count].artist[copy_artist_len] = '\0';
        
        strncpy(songs[count].song, song_start, copy_song_len);
        songs[count].song[copy_song_len] = '\0';
        
        strncpy(songs[count].text, text_start, copy_text_len);
        songs[count].text[copy_text_len] = '\0';
        
        count++;
    }
    
    fclose(file);
    free(buffer);
    *actual_lines = count;  // Retorna quantas linhas foram realmente lidas
}

void count_words_io_optimized(const char* filename, int total_songs, WordCount* word_counts, int* num_words, int world_rank, int world_size) {
    // : Each process processes chunks of lines
    WordCount* local_words = (WordCount*)malloc(MAX_WORDS * sizeof(WordCount));
    int local_num_words = 0;
    
    int current_line = world_rank * LINES_PER_CHUNK; // Start with different chunks for each process
    int processed_count = 0;
    
    printf(": Process %d will process %d lines at a time\n", world_rank, LINES_PER_CHUNK);
    printf("Process %d starting with line %d\n", world_rank, current_line);
    
    //  loop: each process processes chunks, then gets the next available chunk
    while (current_line < total_songs) {
        int chunk_size = (current_line + LINES_PER_CHUNK > total_songs) ? (total_songs - current_line) : LINES_PER_CHUNK;
        
        SongData* songs = (SongData*)malloc(chunk_size * sizeof(SongData));
        int actual_lines;
        
        read_file_chunk_optimized(filename, current_line, chunk_size, songs, &actual_lines);
        
        printf(": Process %d processing chunk starting at line %d (%d lines)\n", 
               world_rank, current_line, actual_lines);
        
        // Process words in this chunk
        for (int i = 0; i < actual_lines; i++) {
            char* text = songs[i].text;
            char* word_start = text;
            
            while (*word_start) {
                // Skip non-alphabetic characters
                while (*word_start && !isalpha(*word_start)) {
                    word_start++;
                }
                
                if (!*word_start) break;
                
                char* word_end = word_start;
                while (*word_end && isalpha(*word_end)) {
                    word_end++;
                }
                
                if (word_end > word_start) {
                    char word[MAX_WORD_LENGTH];
                    int word_len = word_end - word_start;
                    
                    // Skip very short words
                    if (word_len < 2 || word_len > 50) {
                        word_start = word_end;
                        continue;
                    }
                    
                    // Convert to lowercase
                    for (int j = 0; j < word_len && j < MAX_WORD_LENGTH - 1; j++) {
                        char c = word_start[j];
                        word[j] = (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
                    }
                    word[word_len] = '\0';
                    
                    // Search for existing word
                    int found = 0;
                    for (int k = 0; k < local_num_words; k++) {
                        if (strcmp(local_words[k].word, word) == 0) {
                            local_words[k].count++;
                            found = 1;
                            break;
                        }
                    }
                    
                    if (!found && local_num_words < MAX_WORDS - 1) {
                        strcpy(local_words[local_num_words].word, word);
                        local_words[local_num_words].count = 1;
                        local_num_words++;
                    }
                }
                
                word_start = word_end;
            }
        }
        
        processed_count += actual_lines;
        free(songs); // Free chunk immediately
        
        printf(": Process %d completed chunk. Total processed: %d songs, words: %d\n", 
               world_rank, processed_count, local_num_words);
        
        // Get next chunk: current_line += world_size * LINES_PER_CHUNK (round-robin distribution)
        current_line += world_size * LINES_PER_CHUNK;
    }
    
    printf(": Process %d completed. Processed %d songs, found %d unique words.\n", 
           world_rank, processed_count, local_num_words);
    
    // Gather results
    if (world_rank == 0) {
        // Copy local results
        *num_words = local_num_words;
        memcpy(word_counts, local_words, local_num_words * sizeof(WordCount));
        
        // Receive from other processes
        for (int proc = 1; proc < world_size; proc++) {
            int proc_num_words;
            MPI_Recv(&proc_num_words, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            WordCount* proc_words = (WordCount*)malloc(proc_num_words * sizeof(WordCount));
            MPI_Recv(proc_words, proc_num_words * sizeof(WordCount), MPI_BYTE, proc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Merge results
            for (int i = 0; i < proc_num_words; i++) {
                int found = 0;
                for (int j = 0; j < *num_words; j++) {
                    if (strcmp(word_counts[j].word, proc_words[i].word) == 0) {
                        word_counts[j].count += proc_words[i].count;
                        found = 1;
                        break;
                    }
                }
                if (!found && *num_words < MAX_WORDS - 1) {
                    strcpy(word_counts[*num_words].word, proc_words[i].word);
                    word_counts[*num_words].count = proc_words[i].count;
                    (*num_words)++;
                }
            }
            
            free(proc_words);
        }
        
        // Sort by count
        qsort(word_counts, *num_words, sizeof(WordCount), compare_word_counts);
        
        printf(": Word counting completed. Found %d unique words.\n", *num_words);
        printf("Top 10 most frequent words:\n");
        for (int i = 0; i < 10 && i < *num_words; i++) {
            printf("  %d. %s: %d occurrences\n", i + 1, word_counts[i].word, word_counts[i].count);
        }
    } else {
        // Send results to process 0
        MPI_Send(&local_num_words, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_words, local_num_words * sizeof(WordCount), MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    }
    
    free(local_words);
}

void count_artists_io_optimized(const char* filename, int total_songs, ArtistCount* artist_counts, int* num_artists, int world_rank, int world_size) {
    // : Each process processes chunks of lines
    ArtistCount* local_artists = (ArtistCount*)malloc(MAX_ARTISTS * sizeof(ArtistCount));
    int local_num_artists = 0;
    
    int current_line = world_rank * LINES_PER_CHUNK; // Start with different chunks for each process
    int processed_count = 0;
    
    printf(": Process %d will process %d lines at a time\n", world_rank, LINES_PER_CHUNK);
    printf("Process %d starting with line %d\n", world_rank, current_line);
    
    //  loop: each process processes chunks, then gets the next available chunk
    while (current_line < total_songs) {
        int chunk_size = (current_line + LINES_PER_CHUNK > total_songs) ? (total_songs - current_line) : LINES_PER_CHUNK;
        
        SongData* songs = (SongData*)malloc(chunk_size * sizeof(SongData));
        int actual_lines;
        
        read_file_chunk_optimized(filename, current_line, chunk_size, songs, &actual_lines);
        
        printf(": Process %d processing chunk starting at line %d (%d lines)\n", 
               world_rank, current_line, actual_lines);
        
        // Process artists in this chunk
        for (int i = 0; i < actual_lines; i++) {
            char* artist = songs[i].artist;
            
            // Search for existing artist
            int found = 0;
            for (int k = 0; k < local_num_artists; k++) {
                if (strcmp(local_artists[k].artist, artist) == 0) {
                    local_artists[k].song_count++;
                    found = 1;
                    break;
                }
            }
            
            if (!found && local_num_artists < MAX_ARTISTS - 1) {
                strcpy(local_artists[local_num_artists].artist, artist);
                local_artists[local_num_artists].song_count = 1;
                local_num_artists++;
            }
        }
        
        processed_count += actual_lines;
        free(songs); // Free chunk immediately
        
        printf(": Process %d completed chunk. Total processed: %d songs, artists: %d\n", 
               world_rank, processed_count, local_num_artists);
        
        // Get next chunk: current_line += world_size * LINES_PER_CHUNK (round-robin distribution)
        current_line += world_size * LINES_PER_CHUNK;
    }
    
    printf(": Process %d completed. Processed %d songs, found %d unique artists.\n", 
           world_rank, processed_count, local_num_artists);
    
    // Gather results
    if (world_rank == 0) {
        // Copy local results
        *num_artists = local_num_artists;
        memcpy(artist_counts, local_artists, local_num_artists * sizeof(ArtistCount));
        
        // Receive from other processes
        for (int proc = 1; proc < world_size; proc++) {
            int proc_num_artists;
            MPI_Recv(&proc_num_artists, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            ArtistCount* proc_artists = (ArtistCount*)malloc(proc_num_artists * sizeof(ArtistCount));
            MPI_Recv(proc_artists, proc_num_artists * sizeof(ArtistCount), MPI_BYTE, proc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Merge results
            for (int i = 0; i < proc_num_artists; i++) {
                int found = 0;
                for (int j = 0; j < *num_artists; j++) {
                    if (strcmp(artist_counts[j].artist, proc_artists[i].artist) == 0) {
                        artist_counts[j].song_count += proc_artists[i].song_count;
                        found = 1;
                        break;
                    }
                }
                if (!found && *num_artists < MAX_ARTISTS - 1) {
                    strcpy(artist_counts[*num_artists].artist, proc_artists[i].artist);
                    artist_counts[*num_artists].song_count = proc_artists[i].song_count;
                    (*num_artists)++;
                }
            }
            
            free(proc_artists);
        }
        
        // Sort by song count
        qsort(artist_counts, *num_artists, sizeof(ArtistCount), compare_artist_counts);
        
        printf(": Artist counting completed. Found %d unique artists.\n", *num_artists);
        printf("Top 10 artists with most songs:\n");
        for (int i = 0; i < 10 && i < *num_artists; i++) {
            printf("  %d. %s: %d songs\n", i + 1, artist_counts[i].artist, artist_counts[i].song_count);
        }
    } else {
        // Send results to process 0
        MPI_Send(&local_num_artists, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_artists, local_num_artists * sizeof(ArtistCount), MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    }
    
    free(local_artists);
}

void classify_sentiments_io_optimized(const char* filename, int total_songs, int* sentiment_counts, int world_rank, int world_size) {
    (void)world_size; // Suppress unused parameter warning
    int local_sentiment_counts[3] = {0, 0, 0};
    
    if (world_rank == 0) {
        printf(": Classifying sentiments using Ollama...\n");
        printf("Processing only %d songs for LLM analysis (to save time and memory)\n", MAX_LLM_SONGS);
    }
    
    // Only process 0 does LLM classification to avoid conflicts
    if (world_rank == 0) {
        int songs_to_process = (total_songs < MAX_LLM_SONGS) ? total_songs : MAX_LLM_SONGS;
        
        SongData* songs = (SongData*)malloc(songs_to_process * sizeof(SongData));
        int actual_lines;
        
        read_file_chunk_optimized(filename, 0, songs_to_process, songs, &actual_lines);
        
        for (int i = 0; i < actual_lines; i++) {
            char* result = classify_lyrics(songs[i].text);
            if (result) {
                int classification = atoi(result);
                if (classification >= 0 && classification <= 2) {
                    local_sentiment_counts[classification]++;
                }
                free(result);
            }
            
            if ((i + 1) % 5 == 0) {
                printf(": LLM processed %d/%d songs...\n", i + 1, actual_lines);
            }
        }
        
        free(songs);
    }
    
    // Broadcast results to all processes
    MPI_Bcast(local_sentiment_counts, 3, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Copy to output
    memcpy(sentiment_counts, local_sentiment_counts, 3 * sizeof(int));
    
    if (world_rank == 0) {
        printf(": Sentiment classification results:\n");
        printf("Positive: %d songs\n", sentiment_counts[0]);
        printf("Neutral: %d songs\n", sentiment_counts[1]);
        printf("Negative: %d songs\n", sentiment_counts[2]);
    }
}

void print_results(WordCount* word_counts, int num_words, ArtistCount* artist_counts, int num_artists, int* sentiment_counts) {
    printf("\n");
    printf("========================================\n");
    printf("FINAL RESULTS SUMMARY - \n");
    printf("========================================\n");
    
    printf("\n1. WORD COUNTING:\n");
    printf("Total unique words found: %d\n", num_words);
    printf("Top 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < num_words; i++) {
        printf("  %d. %s: %d occurrences\n", i + 1, word_counts[i].word, word_counts[i].count);
    }
    
    printf("\n2. ARTIST ANALYSIS:\n");
    printf("Total unique artists found: %d\n", num_artists);
    printf("Top 10 artists with most songs:\n");
    for (int i = 0; i < 10 && i < num_artists; i++) {
        printf("  %d. %s: %d songs\n", i + 1, artist_counts[i].artist, artist_counts[i].song_count);
    }
    
    printf("\n3. SENTIMENT CLASSIFICATION:\n");
    printf("Positive: %d songs\n", sentiment_counts[0]);
    printf("Neutral: %d songs\n", sentiment_counts[1]);
    printf("Negative: %d songs\n", sentiment_counts[2]);
    
    int total_classified = sentiment_counts[0] + sentiment_counts[1] + sentiment_counts[2];
    if (total_classified > 0) {
        printf("Distribution:\n");
        printf("  Positive: %.1f%%\n", (float)sentiment_counts[0] / total_classified * 100);
        printf("  Neutral: %.1f%%\n", (float)sentiment_counts[1] / total_classified * 100);
        printf("  Negative: %.1f%%\n", (float)sentiment_counts[2] / total_classified * 100);
    }
    
    printf("\n Analysis completed successfully!\n");
}

int compare_word_counts(const void* a, const void* b) {
    WordCount* word_a = (WordCount*)a;
    WordCount* word_b = (WordCount*)b;
    return word_b->count - word_a->count; // Descending order
}

int compare_artist_counts(const void* a, const void* b) {
    ArtistCount* artist_a = (ArtistCount*)a;
    ArtistCount* artist_b = (ArtistCount*)b;
    return artist_b->song_count - artist_a->song_count; // Descending order
}