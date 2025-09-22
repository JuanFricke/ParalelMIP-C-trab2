#include <stdio.h>
#include <string.h>

int main() {
    FILE* file = fopen("spotify_millsongdata.csv", "r");
    if (!file) {
        printf("Error: Could not open file\n");
        return 1;
    }
    
    char line[10000];
    int count = 0;
    
    // Skip header
    if (fgets(line, sizeof(line), file)) {
        printf("Header: %s", line);
    }
    
    // Read first few lines
    while (fgets(line, sizeof(line), file) && count < 5) {
        printf("\nLine %d:\n", count + 1);
        printf("Length: %zu\n", strlen(line));
        
        // Find first 3 commas
        char* comma1 = strchr(line, ',');
        char* comma2 = comma1 ? strchr(comma1 + 1, ',') : NULL;
        char* comma3 = comma2 ? strchr(comma2 + 1, ',') : NULL;
        
        if (comma1 && comma2 && comma3) {
            *comma1 = '\0';
            *comma2 = '\0';
            *comma3 = '\0';
            
            printf("Artist: '%s'\n", line);
            printf("Song: '%s'\n", comma1 + 1);
            printf("Link: '%s'\n", comma2 + 1);
            printf("Text start: '%s'\n", comma3 + 1);
        } else {
            printf("Could not parse line\n");
        }
        
        count++;
    }
    
    fclose(file);
    return 0;
}
