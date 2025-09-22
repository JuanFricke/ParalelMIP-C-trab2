#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 10000
#define MAX_WORD_LENGTH 100
#define MAX_ARTIST_LENGTH 200
#define MAX_SONGS 1000000

// Structure to hold song data
typedef struct {
    char artist[MAX_ARTIST_LENGTH];
    char song[200];
    char text[MAX_LINE_LENGTH];
} SongData;

// Structure for word counting
typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} WordCount;

// Structure for artist counting
typedef struct {
    char artist[MAX_ARTIST_LENGTH];
    int song_count;
} ArtistCount;

// Function prototypes
void parseCSV(const char* filename, SongData* songs, int* total_songs);
void distributeData(SongData* all_songs, int total_songs, SongData* local_songs, int* local_count, int rank, int size);
void countWords(SongData* songs, int count, WordCount* word_counts, int* word_count_size);
void countArtists(SongData* songs, int count, ArtistCount* artist_counts, int* artist_count_size);
void mergeWordCounts(WordCount* local_counts, int local_size, WordCount* global_counts, int* global_size, int rank, int size);
void mergeArtistCounts(ArtistCount* local_counts, int local_size, ArtistCount* global_counts, int* global_size, int rank, int size);
void classifySentiment(SongData* songs, int count, int* sentiment_counts, int rank, int size);
void printResults(WordCount* word_counts, int word_size, ArtistCount* artist_counts, int artist_size, int* sentiment_counts, double total_time);

int main(int argc, char* argv[]) {
    int rank, size;
    double start_time, end_time, total_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
        printf("Starting parallel Spotify data processing with %d processes\n", size);
        start_time = MPI_Wtime();
    }
    
    // Only process 0 reads the CSV file
    SongData* all_songs = NULL;
    int total_songs = 0;
    
    if (rank == 0) {
        all_songs = malloc(MAX_SONGS * sizeof(SongData));
        parseCSV("spotify_millsongdata.csv", all_songs, &total_songs);
        printf("Loaded %d songs from dataset\n", total_songs);
    }
    
    // Broadcast total song count to all processes
    MPI_Bcast(&total_songs, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Calculate local data distribution
    int local_count = total_songs / size;
    int remainder = total_songs % size;
    if (rank < remainder) {
        local_count++;
    }
    
    SongData* local_songs = malloc(local_count * sizeof(SongData));
    
    // Distribute data to all processes
    distributeData(all_songs, total_songs, local_songs, &local_count, rank, size);
    
    if (rank == 0) {
        free(all_songs);
    }
    
    // Task 1: Word counting (40% of grade)
    WordCount* word_counts = malloc(10000 * sizeof(WordCount));
    int word_count_size = 0;
    countWords(local_songs, local_count, word_counts, &word_count_size);
    
    // Merge word counts from all processes
    WordCount* global_word_counts = malloc(50000 * sizeof(WordCount));
    int global_word_size = 0;
    mergeWordCounts(word_counts, word_count_size, global_word_counts, &global_word_size, rank, size);
    
    // Task 2: Artist counting (40% of grade)
    ArtistCount* artist_counts = malloc(10000 * sizeof(ArtistCount));
    int artist_count_size = 0;
    countArtists(local_songs, local_count, artist_counts, &artist_count_size);
    
    // Merge artist counts from all processes
    ArtistCount* global_artist_counts = malloc(10000 * sizeof(ArtistCount));
    int global_artist_size = 0;
    mergeArtistCounts(artist_counts, artist_count_size, global_artist_counts, &global_artist_size, rank, size);
    
    // Task 3: Sentiment classification (20% of grade)
    int sentiment_counts[3] = {0, 0, 0}; // Positive, Neutral, Negative
    classifySentiment(local_songs, local_count, sentiment_counts, rank, size);
    
    // Gather final sentiment counts
    int global_sentiment_counts[3] = {0, 0, 0};
    MPI_Reduce(sentiment_counts, global_sentiment_counts, 3, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Print results on process 0
    if (rank == 0) {
        end_time = MPI_Wtime();
        total_time = end_time - start_time;
        printResults(global_word_counts, global_word_size, global_artist_counts, global_artist_size, global_sentiment_counts, total_time);
    }
    
    // Cleanup
    free(local_songs);
    free(word_counts);
    free(artist_counts);
    free(global_word_counts);
    free(global_artist_counts);
    
    MPI_Finalize();
    return 0;
}

// Parse CSV file and extract song data
void parseCSV(const char* filename, SongData* songs, int* total_songs) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }
    
    char line[MAX_LINE_LENGTH];
    int count = 0;
    
    // Skip header line
    if (fgets(line, sizeof(line), file)) {
        // Header processed
    }
    
    while (fgets(line, sizeof(line), file) && count < MAX_SONGS) {
        // Find the first 3 commas to separate artist, song, link from text
        char* comma1 = strchr(line, ',');
        if (!comma1) continue;
        
        char* comma2 = strchr(comma1 + 1, ',');
        if (!comma2) continue;
        
        char* comma3 = strchr(comma2 + 1, ',');
        if (!comma3) continue;
        
        // Extract artist (between start and first comma)
        *comma1 = '\0';
        char* artist = line;
        // Remove quotes and whitespace
        while (*artist == ' ' || *artist == '\t' || *artist == '"') artist++;
        char* artist_end = artist + strlen(artist) - 1;
        while (artist_end > artist && (*artist_end == ' ' || *artist_end == '\t' || *artist_end == '"' || *artist_end == '\n' || *artist_end == '\r')) {
            *artist_end = '\0';
            artist_end--;
        }
        
        // Extract song (between first and second comma)
        *comma2 = '\0';
        char* song = comma1 + 1;
        while (*song == ' ' || *song == '\t' || *song == '"') song++;
        char* song_end = song + strlen(song) - 1;
        while (song_end > song && (*song_end == ' ' || *song_end == '\t' || *song_end == '"' || *song_end == '\n' || *song_end == '\r')) {
            *song_end = '\0';
            song_end--;
        }
        
        // Extract link (between second and third comma) - we don't use this
        *comma3 = '\0';
        
        // Extract text (after third comma)
        char* text = comma3 + 1;
        
        // Remove leading quote if present
        if (*text == '"') text++;
        
        // For multi-line text, we need to read until we find the closing quote
        // This is a simplified approach - read the rest of the current line
        char* text_end = text + strlen(text) - 1;
        while (text_end > text && (*text_end == '\n' || *text_end == '\r')) {
            text_end--;
        }
        if (*text_end == '"') {
            *text_end = '\0';
        }
        
        // Replace newlines with spaces in the text
        for (char* p = text; *p; p++) {
            if (*p == '\n' || *p == '\r') {
                *p = ' ';
            }
        }
        
        // Copy to song structure
        if (strlen(artist) > 0 && strlen(artist) < MAX_ARTIST_LENGTH) {
            strcpy(songs[count].artist, artist);
        } else {
            strcpy(songs[count].artist, "Unknown");
        }
        
        if (strlen(song) > 0 && strlen(song) < 200) {
            strcpy(songs[count].song, song);
        } else {
            strcpy(songs[count].song, "Unknown");
        }
        
        // Limit text length
        int text_len = strlen(text);
        if (text_len >= MAX_LINE_LENGTH - 1) {
            text[MAX_LINE_LENGTH - 2] = '\0';
        }
        
        if (strlen(text) > 0) {
            strcpy(songs[count].text, text);
        } else {
            strcpy(songs[count].text, "No lyrics");
        }
        
        count++;
    }
    
    *total_songs = count;
    fclose(file);
}

// Distribute data across MPI processes
void distributeData(SongData* all_songs, int total_songs, SongData* local_songs, int* local_count, int rank, int size) {
    int* send_counts = malloc(size * sizeof(int));
    int* send_displs = malloc(size * sizeof(int));
    
    // Calculate send counts and displacements
    int base_count = total_songs / size;
    int remainder = total_songs % size;
    
    for (int i = 0; i < size; i++) {
        send_counts[i] = base_count + (i < remainder ? 1 : 0);
        send_displs[i] = (i == 0) ? 0 : send_displs[i-1] + send_counts[i-1];
    }
    
    // Scatter the data
    MPI_Scatterv(all_songs, send_counts, send_displs, MPI_BYTE,
                 local_songs, *local_count * sizeof(SongData), MPI_BYTE,
                 0, MPI_COMM_WORLD);
    
    free(send_counts);
    free(send_displs);
}

// Count words in song lyrics
void countWords(SongData* songs, int count, WordCount* word_counts, int* word_count_size) {
    *word_count_size = 0;
    
    for (int i = 0; i < count; i++) {
        char* text = songs[i].text;
        char* word = strtok(text, " \t\n\r.,!?;:\"()[]{}");
        
        while (word != NULL) {
            // Convert to lowercase
            for (int j = 0; word[j]; j++) {
                if (word[j] >= 'A' && word[j] <= 'Z') {
                    word[j] = word[j] - 'A' + 'a';
                }
            }
            
            // Skip empty words or very short words
            if (strlen(word) < 2) {
                word = strtok(NULL, " \t\n\r.,!?;:\"()[]{}");
                continue;
            }
            
            // Check if word already exists
            int found = 0;
            for (int k = 0; k < *word_count_size; k++) {
                if (strcmp(word_counts[k].word, word) == 0) {
                    word_counts[k].count++;
                    found = 1;
                    break;
                }
            }
            
            // Add new word if not found
            if (!found && *word_count_size < 10000) {
                strcpy(word_counts[*word_count_size].word, word);
                word_counts[*word_count_size].count = 1;
                (*word_count_size)++;
            }
            
            word = strtok(NULL, " \t\n\r.,!?;:\"()[]{}");
        }
    }
}

// Count songs per artist
void countArtists(SongData* songs, int count, ArtistCount* artist_counts, int* artist_count_size) {
    *artist_count_size = 0;
    
    for (int i = 0; i < count; i++) {
        // Check if artist already exists
        int found = 0;
        for (int j = 0; j < *artist_count_size; j++) {
            if (strcmp(artist_counts[j].artist, songs[i].artist) == 0) {
                artist_counts[j].song_count++;
                found = 1;
                break;
            }
        }
        
        // Add new artist if not found
        if (!found && *artist_count_size < 10000) {
            strcpy(artist_counts[*artist_count_size].artist, songs[i].artist);
            artist_counts[*artist_count_size].song_count = 1;
            (*artist_count_size)++;
        }
    }
}

// Merge word counts from all processes
void mergeWordCounts(WordCount* local_counts, int local_size, WordCount* global_counts, int* global_size, int rank, int size) {
    if (rank == 0) {
        // Process 0 collects all data
        *global_size = local_size;
        memcpy(global_counts, local_counts, local_size * sizeof(WordCount));
        
        for (int i = 1; i < size; i++) {
            WordCount* temp_counts = malloc(10000 * sizeof(WordCount));
            int temp_size;
            
            MPI_Recv(&temp_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(temp_counts, temp_size * sizeof(WordCount), MPI_BYTE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Merge temp_counts into global_counts
            for (int j = 0; j < temp_size; j++) {
                int found = 0;
                for (int k = 0; k < *global_size; k++) {
                    if (strcmp(global_counts[k].word, temp_counts[j].word) == 0) {
                        global_counts[k].count += temp_counts[j].count;
                        found = 1;
                        break;
                    }
                }
                if (!found && *global_size < 50000) {
                    strcpy(global_counts[*global_size].word, temp_counts[j].word);
                    global_counts[*global_size].count = temp_counts[j].count;
                    (*global_size)++;
                }
            }
            
            free(temp_counts);
        }
    } else {
        // Other processes send their data to process 0
        MPI_Send(&local_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_counts, local_size * sizeof(WordCount), MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    }
}

// Merge artist counts from all processes
void mergeArtistCounts(ArtistCount* local_counts, int local_size, ArtistCount* global_counts, int* global_size, int rank, int size) {
    if (rank == 0) {
        // Process 0 collects all data
        *global_size = local_size;
        memcpy(global_counts, local_counts, local_size * sizeof(ArtistCount));
        
        for (int i = 1; i < size; i++) {
            ArtistCount* temp_counts = malloc(10000 * sizeof(ArtistCount));
            int temp_size;
            
            MPI_Recv(&temp_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(temp_counts, temp_size * sizeof(ArtistCount), MPI_BYTE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Merge temp_counts into global_counts
            for (int j = 0; j < temp_size; j++) {
                int found = 0;
                for (int k = 0; k < *global_size; k++) {
                    if (strcmp(global_counts[k].artist, temp_counts[j].artist) == 0) {
                        global_counts[k].song_count += temp_counts[j].song_count;
                        found = 1;
                        break;
                    }
                }
                if (!found && *global_size < 10000) {
                    strcpy(global_counts[*global_size].artist, temp_counts[j].artist);
                    global_counts[*global_size].song_count = temp_counts[j].song_count;
                    (*global_size)++;
                }
            }
            
            free(temp_counts);
        }
    } else {
        // Other processes send their data to process 0
        MPI_Send(&local_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_counts, local_size * sizeof(ArtistCount), MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    }
}

// Simple sentiment classification based on keyword analysis
void classifySentiment(SongData* songs, int count, int* sentiment_counts, int rank, int size) {
    // Simple keyword-based sentiment analysis
    char* positive_words[] = {"love", "happy", "joy", "beautiful", "wonderful", "amazing", "great", "good", "best", "smile", "laugh", "dream", "hope", "peace", "free"};
    char* negative_words[] = {"hate", "sad", "pain", "hurt", "cry", "death", "die", "kill", "bad", "worst", "angry", "fear", "scared", "lonely", "broken"};
    
    int pos_words_count = sizeof(positive_words) / sizeof(positive_words[0]);
    int neg_words_count = sizeof(negative_words) / sizeof(negative_words[0]);
    
    for (int i = 0; i < count; i++) {
        char text_copy[MAX_LINE_LENGTH];
        strcpy(text_copy, songs[i].text);
        
        // Convert to lowercase
        for (int j = 0; text_copy[j]; j++) {
            if (text_copy[j] >= 'A' && text_copy[j] <= 'Z') {
                text_copy[j] = text_copy[j] - 'A' + 'a';
            }
        }
        
        int positive_score = 0;
        int negative_score = 0;
        
        // Count positive words
        for (int j = 0; j < pos_words_count; j++) {
            char* found = strstr(text_copy, positive_words[j]);
            while (found != NULL) {
                positive_score++;
                found = strstr(found + 1, positive_words[j]);
            }
        }
        
        // Count negative words
        for (int j = 0; j < neg_words_count; j++) {
            char* found = strstr(text_copy, negative_words[j]);
            while (found != NULL) {
                negative_score++;
                found = strstr(found + 1, negative_words[j]);
            }
        }
        
        // Classify sentiment
        if (positive_score > negative_score) {
            sentiment_counts[0]++; // Positive
        } else if (negative_score > positive_score) {
            sentiment_counts[2]++; // Negative
        } else {
            sentiment_counts[1]++; // Neutral
        }
    }
}

// Print final results
void printResults(WordCount* word_counts, int word_size, ArtistCount* artist_counts, int artist_size, int* sentiment_counts, double total_time) {
    printf("\n=== PARALLEL SPOTIFY DATA PROCESSING RESULTS ===\n");
    printf("Total execution time: %.2f seconds\n\n", total_time);
    
    // Sort and display top 20 words
    printf("=== TOP 20 MOST FREQUENT WORDS ===\n");
    for (int i = 0; i < word_size - 1; i++) {
        for (int j = i + 1; j < word_size; j++) {
            if (word_counts[i].count < word_counts[j].count) {
                WordCount temp = word_counts[i];
                word_counts[i] = word_counts[j];
                word_counts[j] = temp;
            }
        }
    }
    
    for (int i = 0; i < 20 && i < word_size; i++) {
        printf("%d. %s: %d occurrences\n", i + 1, word_counts[i].word, word_counts[i].count);
    }
    
    // Sort and display top 20 artists
    printf("\n=== TOP 20 ARTISTS WITH MOST SONGS ===\n");
    for (int i = 0; i < artist_size - 1; i++) {
        for (int j = i + 1; j < artist_size; j++) {
            if (artist_counts[i].song_count < artist_counts[j].song_count) {
                ArtistCount temp = artist_counts[i];
                artist_counts[i] = artist_counts[j];
                artist_counts[j] = temp;
            }
        }
    }
    
    for (int i = 0; i < 20 && i < artist_size; i++) {
        printf("%d. %s: %d songs\n", i + 1, artist_counts[i].artist, artist_counts[i].song_count);
    }
    
    // Display sentiment analysis results
    printf("\n=== SENTIMENT ANALYSIS RESULTS ===\n");
    printf("Positive songs: %d\n", sentiment_counts[0]);
    printf("Neutral songs: %d\n", sentiment_counts[1]);
    printf("Negative songs: %d\n", sentiment_counts[2]);
    
    int total_songs = sentiment_counts[0] + sentiment_counts[1] + sentiment_counts[2];
    if (total_songs > 0) {
        printf("\nSentiment distribution:\n");
        printf("Positive: %.1f%%\n", (double)sentiment_counts[0] / total_songs * 100);
        printf("Neutral: %.1f%%\n", (double)sentiment_counts[1] / total_songs * 100);
        printf("Negative: %.1f%%\n", (double)sentiment_counts[2] / total_songs * 100);
    }
    
    printf("\n=== PERFORMANCE METRICS ===\n");
    printf("Total words processed: %d unique words\n", word_size);
    printf("Total artists processed: %d unique artists\n", artist_size);
    printf("Total songs processed: %d\n", total_songs);
    printf("Processing rate: %.2f songs/second\n", total_songs / total_time);
}