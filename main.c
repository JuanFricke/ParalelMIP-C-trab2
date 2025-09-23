#include <stdio.h>
#include <stdlib.h>
#include "ollama_client.h"

int main() {
    printf("Simple Lyrics Classification Example\n");
    printf("===================================\n\n");
    
    // Array to count classifications: [0] = class 1, [1] = class 2, [2] = class 3
    int classification_count[3] = {0, 0, 0};
    char *result;
    
    // Test 1: Positive lyrics
    printf("Test 1: Positive Lyrics\n");
    printf("------------------------\n");
    const char *positive_lyrics = "Look at her face, it's a wonderful face\n"
                                 "And it means something special to me\n"
                                 "Look at the way that she smiles when she sees me\n"
                                 "How lucky can one fellow be?\n\n"
                                 "She's just my kind of girl, she makes me feel fine\n"
                                 "Who could ever believe that she could be mine?";
    
    printf("Lyrics: %s\n", positive_lyrics);
    result = classify_lyrics(positive_lyrics);
    printf("%s\n\n", result);
    classification_count[atoi(result)]++;
    free(result);
    
    // Test 2: Negative lyrics
    printf("Test 2: Negative Lyrics\n");
    printf("------------------------\n");
    const char *negative_lyrics = "I'm feeling so alone, the darkness surrounds me\n"
                                 "Pain and sorrow fill my heart, I can't break free\n"
                                 "Everything is falling apart, there's no hope for me\n"
                                 "The world is cold and empty, I just want to flee";
    
    printf("Lyrics: %s\n", negative_lyrics);
    result = classify_lyrics(negative_lyrics);
    printf("%s\n\n", result);
    classification_count[atoi(result)]++;
    free(result);
    
    // Test 3: Neutral lyrics
    printf("Test 3: Neutral Lyrics\n");
    printf("-----------------------\n");
    const char *neutral_lyrics = "The clock strikes twelve, another day begins\n"
                                "Walking down the street, watching people pass\n"
                                "The weather is okay, not too hot or cold\n"
                                "Just another ordinary day, nothing to behold";
    
    printf("Lyrics: %s\n", neutral_lyrics);
    result = classify_lyrics(neutral_lyrics);
    printf("%s\n\n", result);
    classification_count[atoi(result)]++;
    free(result);

    // Test 4: Bunda Lyrics
    printf("Test 4: Bunda Lyrics\n");
    printf("-----------------------\n");
    const char *bad_lyrics = "funcking good pussy eating BUCETAAAAAAAAAAAA MERDA TURBAAAAAAAAAAAAAAA shake ass pussy fuck pussy boobs";
    
    printf("Lyrics: %s\n", bad_lyrics);
    result = classify_lyrics(bad_lyrics);
    printf("%s\n\n", result);
    classification_count[atoi(result)]++;
    free(result);
    
    // Display results table
    printf("Classification Results:\n");
    printf("======================\n");
    printf("Class: 0 | 1 | 2\n");
    printf("Count: %d | %d | %d\n", classification_count[0], classification_count[1], classification_count[2]);

    printf("\nClassification completed!\n");
    return 0;
}