#ifndef OLLAMA_CLIENT_H
#define OLLAMA_CLIENT_H

#define _GNU_SOURCE  // Habilita strdup e outras extensões GNU

// Inclusão das bibliotecas necessárias
#include <stdio.h>    // Para entrada e saída padrão
#include <stdlib.h>   // Para funções de alocação de memória
#include <string.h>   // Para manipulação de strings
#include <curl/curl.h>  // Para requisições HTTP

// Endpoint da API do Ollama (padrão local)
#define OLLAMA_DEFAULT_URL "http://localhost:11434"

// Estrutura para resposta da API do Ollama
typedef struct {
    char *response;      // O texto da resposta real
    char *model;         // Modelo usado para gerar a resposta
    int done;            // Se a resposta está completa
    char *error;         // Mensagem de erro se houver
    size_t response_len; // Tamanho da resposta
} OllamaResponse;

// Estrutura para requisição à API do Ollama
typedef struct {
    char *model;         // Modelo a usar (ex: "llama2", "codellama")
    char *prompt;        // Prompt de entrada
    int stream;          // Se deve fazer streaming da resposta
    char *format;        // Formato da resposta (opcional)
} OllamaRequest;

// Estrutura de configuração
typedef struct {
    char *url;           // URL do servidor Ollama
    int timeout;         // Timeout da requisição em segundos
    int verbose;         // Log verboso
} OllamaConfig;

// Declarações das funções

/**
 * Inicializa a configuração do Ollama com valores padrão
 * @return Ponteiro para estrutura OllamaConfig alocada
 */
OllamaConfig* ollama_config_init(void);

/**
 * Libera a memória da configuração do Ollama
 * @param config Ponteiro para OllamaConfig a ser liberado
 */
void ollama_config_free(OllamaConfig *config);

/**
 * Libera a memória da requisição do Ollama
 * @param request Ponteiro para OllamaRequest a ser liberado
 */
void ollama_request_free(OllamaRequest *request);

/**
 * Inicializa a estrutura de resposta do Ollama
 * @return Ponteiro para estrutura OllamaResponse alocada
 */
OllamaResponse* ollama_response_init(void);

/**
 * Libera a memória da resposta do Ollama
 * @param response Ponteiro para OllamaResponse a ser liberado
 */
void ollama_response_free(OllamaResponse *response);

/**
 * Envia uma requisição para o Ollama e obtém a resposta
 * @param config Configuração do Ollama
 * @param request Requisição a ser enviada
 * @param response Estrutura de resposta a ser preenchida
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int ollama_send_request(const OllamaConfig *config, 
                       const OllamaRequest *request, 
                       OllamaResponse *response);

/**
 * Inicializa requisição do Ollama para classificação de sentimento de letras de música
 * @param model Nome do modelo a usar
 * @param lyrics Letras da música a classificar
 * @return Ponteiro para estrutura OllamaRequest alocada com prompt pré-definido
 */
OllamaRequest* ollama_request_init_classification(const char *model, const char *lyrics);

/**
 * Função simplificada para classificar o sentimento das letras
 * @param lyrics Letras da música a classificar
 * @return Resultado da classificação: "0" (Positivo), "1" (Neutro), ou "2" (Negativo)
 */
char* classify_lyrics(const char *lyrics);

#endif // OLLAMA_CLIENT_H