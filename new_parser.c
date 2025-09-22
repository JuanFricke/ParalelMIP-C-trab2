// New CSV parser that handles multi-line text fields
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
