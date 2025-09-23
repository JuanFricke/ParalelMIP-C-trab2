#include "ollama_client.h"
#include <json-c/json.h>

// Constants
#define OLLAMA_URL "http://localhost:11434"
#define OLLAMA_MODEL_NAME "wizard-vicuna-uncensored:7b"
#define OLLAMA_PRE_PROMPT "You are a sentiment classifier for song lyrics. " \
                         "Analyze the provided lyrics and classify the sentiment as: " \
                         "0: \"Positive\", 1: \"Neutral\" or 2: \"Negative\". " \
                         "Answer ONLY with one of these three numbers, use only one number without additional explanations or words your awnsware needs to be only one character long.\n\n" \
                         "Lyrics to classify:\n"

// Callback function for libcurl to write response data
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    OllamaResponse *response = (OllamaResponse *)userp;
    
    char *ptr = realloc(response->response, response->response_len + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Memory allocation error\n");
        return 0;
    }
    
    response->response = ptr;
    memcpy(&(response->response[response->response_len]), contents, realsize);
    response->response_len += realsize;
    response->response[response->response_len] = 0;
    
    return realsize;
}

OllamaConfig* ollama_config_init(void) {
    OllamaConfig *config = malloc(sizeof(OllamaConfig));
    config->url = strdup(OLLAMA_DEFAULT_URL);
    config->timeout = 30;
    config->verbose = 0;
    return config;
}

void ollama_config_free(OllamaConfig *config) {
    if (config) {
        free(config->url);
        free(config);
    }
}

OllamaRequest* ollama_request_init(const char *model, const char *prompt) {
    OllamaRequest *request = malloc(sizeof(OllamaRequest));
    request->model = strdup(model);
    request->prompt = strdup(prompt);
    request->stream = 0;
    request->format = NULL;
    return request;
}

void ollama_request_free(OllamaRequest *request) {
    if (request) {
        free(request->model);
        free(request->prompt);
        if (request->format) {
            free(request->format);
        }
        free(request);
    }
}

OllamaRequest* ollama_request_init_classification(const char *model, const char *lyrics) {
    OllamaRequest *request = malloc(sizeof(OllamaRequest));
    request->model = strdup(model);
    request->stream = 0;
    request->format = NULL;
    

    // engenharia de prompt é merda mesmo
    const char *pre_prompt = OLLAMA_PRE_PROMPT;
    
    size_t pre_prompt_len = strlen(pre_prompt);
    size_t lyrics_len = strlen(lyrics);
    size_t total_len = pre_prompt_len + lyrics_len + 1;
    
    request->prompt = malloc(total_len);
    strcpy(request->prompt, pre_prompt);
    strcat(request->prompt, lyrics);
    
    return request;
}

OllamaResponse* ollama_response_init(void) {
    OllamaResponse *response = malloc(sizeof(OllamaResponse));
    response->response = NULL;
    response->model = NULL;
    response->done = 0;
    response->error = NULL;
    response->response_len = 0;
    return response;
}

void ollama_response_free(OllamaResponse *response) {
    if (response) {
        free(response->response);
        free(response->model);
        free(response->error);
        free(response);
    }
}

int ollama_send_request(const OllamaConfig *config, 
                       const OllamaRequest *request, 
                       OllamaResponse *response) {
    CURL *curl;
    CURLcode res;
    struct json_object *json_obj, *model_obj, *prompt_obj, *stream_obj;
    char *json_string;
    
    curl = curl_easy_init();
    
    // Create JSON request
    json_obj = json_object_new_object();
    model_obj = json_object_new_string(request->model);
    prompt_obj = json_object_new_string(request->prompt);
    stream_obj = json_object_new_boolean(request->stream);
    
    json_object_object_add(json_obj, "model", model_obj);
    json_object_object_add(json_obj, "prompt", prompt_obj);
    json_object_object_add(json_obj, "stream", stream_obj);
    
    if (request->format) {
        struct json_object *format_obj = json_object_new_string(request->format);
        json_object_object_add(json_obj, "format", format_obj);
    }
    
    json_string = (char *)json_object_to_json_string(json_obj);
    
    // Set up curl options
    char url[512];
    snprintf(url, sizeof(url), "%s/api/generate", config->url);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config->timeout);
    
    if (config->verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    
    // Perform the request
    res = curl_easy_perform(curl);
    
    // Check for errors
    if (res != CURLE_OK) {
        response->error = strdup(curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        json_object_put(json_obj);
        return -1;
    }
    
    // Parse response JSON
    if (response->response) {
        struct json_object *response_json = json_tokener_parse(response->response);
        if (response_json) {
            struct json_object *response_text, *model_name, *done_flag, *error_msg;
            
            if (json_object_object_get_ex(response_json, "response", &response_text)) {
                const char *text = json_object_get_string(response_text);
                if (text) {
                    free(response->response);
                    response->response = strdup(text);
                    response->response_len = strlen(text);
                }
            }
            
            if (json_object_object_get_ex(response_json, "model", &model_name)) {
                const char *model = json_object_get_string(model_name);
                if (model) {
                    response->model = strdup(model);
                }
            }
            
            if (json_object_object_get_ex(response_json, "done", &done_flag)) {
                response->done = json_object_get_boolean(done_flag);
            }
            
            if (json_object_object_get_ex(response_json, "error", &error_msg)) {
                const char *error = json_object_get_string(error_msg);
                if (error) {
                    response->error = strdup(error);
                }
            }
            
            json_object_put(response_json);
        }
    }
    
    curl_easy_cleanup(curl);
    json_object_put(json_obj);
    
    return 0;
}

void ollama_print_response(const OllamaResponse *response) {
    if (!response) {
        printf("No response to print\n");
        return;
    }
    
    printf("=== Ollama Response ===\n");
    if (response->model) {
        printf("Model: %s\n", response->model);
    }
    if (response->response) {
        printf("Response: %s\n", response->response);
    }
    if (response->error) {
        printf("Error: %s\n", response->error);
    }
    printf("Done: %s\n", response->done ? "Yes" : "No");
    printf("======================\n");
}

int ollama_check_server(const OllamaConfig *config) {
    if (!config) {
        return 0;
    }
    
    CURL *curl;
    CURLcode res;
    long response_code;
    
    curl = curl_easy_init();
    if (!curl) {
        return 0;
    }
    
    char url[512];
    snprintf(url, sizeof(url), "%s/api/tags", config->url);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  // HEAD request only
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    
    res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_cleanup(curl);
        return (response_code == 200) ? 1 : 0;
    }
    
    curl_easy_cleanup(curl);
    return 0;
}

char* classify_lyrics(const char *lyrics) {
    OllamaConfig *config = ollama_config_init();
    OllamaRequest *request = ollama_request_init_classification(OLLAMA_MODEL_NAME, lyrics);
    OllamaResponse *response = ollama_response_init();
    
    ollama_send_request(config, request, response);
    
    char *result = strdup(response->response);
    
    ollama_response_free(response);
    ollama_request_free(request);
    ollama_config_free(config);
    
    return result;
}
