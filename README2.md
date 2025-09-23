# Ollama C Client

A simple and easy-to-use C library for interacting with Ollama (local LLM server).

## Features

- **Standardized API**: Clean, consistent interface for all Ollama operations
- **Memory Management**: Automatic memory allocation and cleanup
- **Error Handling**: Comprehensive error checking and reporting
- **JSON Support**: Built-in JSON request/response handling
- **Server Health Check**: Verify Ollama server availability
- **Configurable**: Customizable timeouts, URLs, and verbosity

## Dependencies

- **libcurl**: For HTTP requests
- **json-c**: For JSON parsing
- **Ollama server**: Must be running locally
- **GNU C Library**: Uses GNU extensions (strdup, etc.)

### Installing Dependencies

#### Ubuntu/Debian:
```bash
sudo apt-get install libcurl4-openssl-dev libjson-c-dev
```

#### Fedora/RHEL:
```bash
sudo dnf install libcurl-devel json-c-devel
```

## Building

```bash
# Build the project
make

# Or build with debug symbols
make debug

# Or build optimized release
make release
```

## Usage

### Basic Example

```c
#include "ollama_client.h"

int main() {
    // Initialize configuration
    OllamaConfig *config = ollama_config_init();
    
    // Check if server is running
    if (!ollama_check_server(config)) {
        printf("Ollama server not running!\n");
        return 1;
    }
    
    // Create request
    OllamaRequest *request = ollama_request_init("llama2", "Hello, how are you?");
    
    // Initialize response
    OllamaResponse *response = ollama_response_init();
    
    // Send request
    int result = ollama_send_request(config, request, response);
    
    if (result == 0) {
        ollama_print_response(response);
    }
    
    // Clean up
    ollama_response_free(response);
    ollama_request_free(request);
    ollama_config_free(config);
    
    return 0;
}
```

### Advanced Usage

```c
// Custom configuration
OllamaConfig *config = ollama_config_init();
config->timeout = 60;  // 60 second timeout
config->verbose = 1;   // Enable verbose logging

// Custom request with format
OllamaRequest *request = ollama_request_init("codellama", "Write a C function");
request->format = strdup("json");  // Request JSON format

// Send and handle response
OllamaResponse *response = ollama_response_init();
int result = ollama_send_request(config, request, response);

if (result == 0 && !response->error) {
    printf("Model: %s\n", response->model);
    printf("Response: %s\n", response->response);
    printf("Complete: %s\n", response->done ? "Yes" : "No");
}
```

## API Reference

### Structures

#### OllamaConfig
- `char *url`: Ollama server URL (default: "http://localhost:11434")
- `int timeout`: Request timeout in seconds (default: 30)
- `int verbose`: Enable verbose logging (default: 0)

#### OllamaRequest
- `char *model`: Model name (e.g., "llama2", "codellama")
- `char *prompt`: Input prompt text
- `int stream`: Enable streaming (default: 0)
- `char *format`: Response format (optional)

#### OllamaResponse
- `char *response`: The actual response text
- `char *model`: Model used for generation
- `int done`: Whether response is complete
- `char *error`: Error message if any
- `size_t response_len`: Length of response

### Functions

#### Configuration
- `OllamaConfig* ollama_config_init(void)`: Initialize config with defaults
- `void ollama_config_free(OllamaConfig *config)`: Free config memory

#### Request/Response
- `OllamaRequest* ollama_request_init(const char *model, const char *prompt)`: Create request
- `void ollama_request_free(OllamaRequest *request)`: Free request memory
- `OllamaResponse* ollama_response_init(void)`: Initialize response
- `void ollama_response_free(OllamaResponse *response)`: Free response memory

#### Communication
- `int ollama_send_request(const OllamaConfig *config, const OllamaRequest *request, OllamaResponse *response)`: Send request
- `int ollama_check_server(const OllamaConfig *config)`: Check server availability
- `void ollama_print_response(const OllamaResponse *response)`: Print formatted response

## Running the Example

1. Start Ollama server:
   ```bash
   ollama serve
   ```

2. Pull a model (if not already available):
   ```bash
   ollama pull llama2
   ```

3. Build and run:
   ```bash
   make
   ./ollama_client
   ```

## Error Handling

The library provides comprehensive error handling:

- **Memory allocation errors**: Check return values of init functions
- **Network errors**: Check return value of `ollama_send_request()`
- **Server errors**: Check `response->error` field
- **Server availability**: Use `ollama_check_server()` before sending requests

## Notes

- Always call the corresponding `*_free()` functions to prevent memory leaks
- The library uses blocking HTTP requests (non-streaming by default)
- JSON responses are automatically parsed
- Server health checks use HEAD requests for efficiency
