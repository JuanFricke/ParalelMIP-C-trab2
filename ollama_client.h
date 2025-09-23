#ifndef OLLAMA_CLIENT_H
#define OLLAMA_CLIENT_H

#define _GNU_SOURCE  // Enable strdup and other GNU extensions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Ollama API endpoint (default local)
#define OLLAMA_DEFAULT_URL "http://localhost:11434"

// Response structure for Ollama API
typedef struct {
    char *response;      // The actual response text
    char *model;         // Model used
    int done;            // Whether the response is complete
    char *error;         // Error message if any
    size_t response_len; // Length of response
} OllamaResponse;

// Request structure for Ollama API
typedef struct {
    char *model;         // Model to use (e.g., "llama2", "codellama")
    char *prompt;        // Input prompt
    int stream;          // Whether to stream the response
    char *format;        // Response format (optional)
} OllamaRequest;

// Configuration structure
typedef struct {
    char *url;           // Ollama server URL
    int timeout;         // Request timeout in seconds
    int verbose;         // Verbose logging
} OllamaConfig;

// Function declarations

/**
 * Initialize Ollama configuration with default values
 * @return Pointer to allocated OllamaConfig structure
 */
OllamaConfig* ollama_config_init(void);

/**
 * Free Ollama configuration memory
 * @param config Pointer to OllamaConfig to free
 */
void ollama_config_free(OllamaConfig *config);

/**
 * Initialize Ollama request structure
 * @param model Model name to use
 * @param prompt Input prompt
 * @return Pointer to allocated OllamaRequest structure
 */
OllamaRequest* ollama_request_init(const char *model, const char *prompt);

/**
 * Free Ollama request memory
 * @param request Pointer to OllamaRequest to free
 */
void ollama_request_free(OllamaRequest *request);

/**
 * Initialize Ollama response structure
 * @return Pointer to allocated OllamaResponse structure
 */
OllamaResponse* ollama_response_init(void);

/**
 * Free Ollama response memory
 * @param response Pointer to OllamaResponse to free
 */
void ollama_response_free(OllamaResponse *response);

/**
 * Send a request to Ollama and get response
 * @param config Ollama configuration
 * @param request Request to send
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int ollama_send_request(const OllamaConfig *config, 
                       const OllamaRequest *request, 
                       OllamaResponse *response);

/**
 * Print Ollama response in a formatted way
 * @param response Response to print
 */
void ollama_print_response(const OllamaResponse *response);

/**
 * Check if Ollama server is running
 * @param config Ollama configuration
 * @return 1 if running, 0 if not
 */
int ollama_check_server(const OllamaConfig *config);

/**
 * Initialize Ollama request for sentiment classification of music lyrics
 * @param model Model name to use
 * @param lyrics Music lyrics to classify
 * @return Pointer to allocated OllamaRequest structure with pre-prompt
 */
OllamaRequest* ollama_request_init_classification(const char *model, const char *lyrics);

/**
 * Simple function to classify lyrics sentiment
 * @param lyrics Music lyrics to classify
 * @return Classification result: "Positive", "Neutral", or "Negative"
 */
char* classify_lyrics(const char *lyrics);

#endif // OLLAMA_CLIENT_H
