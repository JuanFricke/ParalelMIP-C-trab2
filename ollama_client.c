#include "ollama_client.h"
#include <json-c/json.h>  // Biblioteca para manipulação de JSON

// Constantes para configuração do Ollama
#define OLLAMA_URL "http://localhost:11434"  // URL do servidor Ollama local
#define OLLAMA_MODEL_NAME "wizard-vicuna-uncensored:7b"  // Nome do modelo de IA a ser usado
#define OLLAMA_PRE_PROMPT "You are a sentiment classifier for song lyrics. " \
                         "Analyze the provided lyrics and classify the sentiment as: " \
                         "0: \"Positive\", 1: \"Neutral\" or 2: \"Negative\". " \
                         "Answer ONLY with one of these three numbers, use only one number without additional explanations or words your awnsware needs to be only one character long.\n\n" \
                         "Lyrics to classify:\n"  // Prompt pré-definido para classificação de sentimento

// Função callback para libcurl escrever dados de resposta
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;  // Tamanho real dos dados recebidos
    OllamaResponse *response = (OllamaResponse *)userp;  // Estrutura de resposta
    
    // Realoca memória para acomodar os novos dados
    char *ptr = realloc(response->response, response->response_len + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Erro de alocação de memória\n");
        return 0;
    }
    
    response->response = ptr;
    // Copia os novos dados para o final da resposta existente
    memcpy(&(response->response[response->response_len]), contents, realsize);
    response->response_len += realsize;
    response->response[response->response_len] = 0;  // Termina a string com null
    
    return realsize;  // Retorna o número de bytes processados
}

// Inicializa a configuração do Ollama com valores padrão
OllamaConfig* ollama_config_init(void) {
    OllamaConfig *config = malloc(sizeof(OllamaConfig));
    config->url = strdup(OLLAMA_DEFAULT_URL);  // URL padrão do servidor
    config->timeout = 30;  // Timeout de 30 segundos
    config->verbose = 0;   // Modo não verboso por padrão
    return config;
}

// Libera a memória alocada para a configuração do Ollama
void ollama_config_free(OllamaConfig *config) {
    if (config) {
        free(config->url);  // Libera a string da URL
        free(config);       // Libera a estrutura principal
    }
}

// Inicializa uma requisição para classificação de sentimento
OllamaRequest* ollama_request_init_classification(const char *model, const char *lyrics) {
    OllamaRequest *request = malloc(sizeof(OllamaRequest));
    request->model = strdup(model);  // Nome do modelo a ser usado
    request->stream = 0;             // Não usar streaming
    request->format = NULL;          // Sem formato específico
    
    const char *pre_prompt = OLLAMA_PRE_PROMPT;  // Prompt pré-definido
    
    // Calcula o tamanho total necessário para o prompt completo
    size_t pre_prompt_len = strlen(pre_prompt);
    size_t lyrics_len = strlen(lyrics);
    size_t total_len = pre_prompt_len + lyrics_len + 1;
    
    // Aloca memória e monta o prompt completo
    request->prompt = malloc(total_len);
    strcpy(request->prompt, pre_prompt);  // Copia o prompt base
    strcat(request->prompt, lyrics);      // Adiciona as letras da música
    
    return request;
}

// Libera a memória alocada para uma requisição do Ollama
void ollama_request_free(OllamaRequest *request) {
    if (request) {
        free(request->model);   // Libera o nome do modelo
        free(request->prompt);  // Libera o prompt
        if (request->format) {  // Libera o formato se existir
            free(request->format);
        }
        free(request);  // Libera a estrutura principal
    }
}

// Inicializa uma estrutura de resposta do Ollama
OllamaResponse* ollama_response_init(void) {
    OllamaResponse *response = malloc(sizeof(OllamaResponse));
    response->response = NULL;     // Resposta ainda não recebida
    response->model = NULL;        // Modelo usado ainda não definido
    response->done = 0;            // Resposta ainda não completa
    response->error = NULL;        // Sem erro inicial
    response->response_len = 0;    // Tamanho inicial zero
    return response;
}

// Libera a memória alocada para uma resposta do Ollama
void ollama_response_free(OllamaResponse *response) {
    if (response) {
        free(response->response);  // Libera o texto da resposta
        free(response->model);     // Libera o nome do modelo
        free(response->error);     // Libera a mensagem de erro se existir
        free(response);            // Libera a estrutura principal
    }
}

// Envia uma requisição para o Ollama e obtém a resposta
int ollama_send_request(const OllamaConfig *config, 
                       const OllamaRequest *request, 
                       OllamaResponse *response) {
    CURL *curl;  // Handle do libcurl
    CURLcode res;  // Código de resultado do curl
    struct json_object *json_obj, *model_obj, *prompt_obj, *stream_obj;  // Objetos JSON
    char *json_string;  // String JSON final
    
    curl = curl_easy_init();  // Inicializa o curl
    
    // Cria a requisição JSON
    json_obj = json_object_new_object();  // Objeto JSON principal
    model_obj = json_object_new_string(request->model);  // Nome do modelo
    prompt_obj = json_object_new_string(request->prompt);  // Prompt da requisição
    stream_obj = json_object_new_boolean(request->stream);  // Flag de streaming
    
    // Adiciona os campos ao objeto JSON principal
    json_object_object_add(json_obj, "model", model_obj);
    json_object_object_add(json_obj, "prompt", prompt_obj);
    json_object_object_add(json_obj, "stream", stream_obj);
    
    // Adiciona formato se especificado
    if (request->format) {
        struct json_object *format_obj = json_object_new_string(request->format);
        json_object_object_add(json_obj, "format", format_obj);
    }
    
    // Converte o objeto JSON para string
    json_string = (char *)json_object_to_json_string(json_obj);
    
    // Configura as opções do curl
    char url[512];
    snprintf(url, sizeof(url), "%s/api/generate", config->url);  // Monta a URL da API
    
    curl_easy_setopt(curl, CURLOPT_URL, url);  // Define a URL de destino
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);  // Define os dados POST
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);  // Função de callback para escrita
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);  // Dados para o callback
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config->timeout);  // Timeout da requisição
    
    // Habilita modo verboso se configurado
    if (config->verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    
    // Executa a requisição
    res = curl_easy_perform(curl);
    
    // Verifica se houve erros
    if (res != CURLE_OK) {
        response->error = strdup(curl_easy_strerror(res));  // Salva a mensagem de erro
        curl_easy_cleanup(curl);  // Limpa o curl
        json_object_put(json_obj);  // Libera o objeto JSON
        return -1;  // Retorna erro
    }
    
    // Analisa a resposta JSON
    if (response->response) {
        struct json_object *response_json = json_tokener_parse(response->response);
        if (response_json) {
            struct json_object *response_text, *model_name, *done_flag, *error_msg;
            
            // Extrai o texto da resposta
            if (json_object_object_get_ex(response_json, "response", &response_text)) {
                const char *text = json_object_get_string(response_text);
                if (text) {
                    free(response->response);  // Libera a resposta original
                    response->response = strdup(text);  // Salva apenas o texto
                    response->response_len = strlen(text);
                }
            }
            
            // Extrai o nome do modelo usado
            if (json_object_object_get_ex(response_json, "model", &model_name)) {
                const char *model = json_object_get_string(model_name);
                if (model) {
                    response->model = strdup(model);
                }
            }
            
            // Extrai a flag de conclusão
            if (json_object_object_get_ex(response_json, "done", &done_flag)) {
                response->done = json_object_get_boolean(done_flag);
            }
            
            // Extrai mensagem de erro se existir
            if (json_object_object_get_ex(response_json, "error", &error_msg)) {
                const char *error = json_object_get_string(error_msg);
                if (error) {
                    response->error = strdup(error);
                }
            }
            
            json_object_put(response_json);  // Libera o objeto JSON da resposta
        }
    }
    
    curl_easy_cleanup(curl);  // Limpa o handle do curl
    json_object_put(json_obj);  // Libera o objeto JSON da requisição
    
    return 0;  // Sucesso
}

// Função simplificada para classificar o sentimento das letras de música
char* classify_lyrics(const char *lyrics) {
    OllamaConfig *config = ollama_config_init();  // Inicializa configuração
    OllamaRequest *request = ollama_request_init_classification(OLLAMA_MODEL_NAME, lyrics);  // Cria requisição
    OllamaResponse *response = ollama_response_init();  // Inicializa resposta
    
    ollama_send_request(config, request, response);  // Envia requisição
    
    char *result = strdup(response->response);  // Copia o resultado
    
    // Libera toda a memória alocada
    ollama_response_free(response);
    ollama_request_free(request);
    ollama_config_free(config);
    
    return result;  // Retorna o resultado da classificação
}