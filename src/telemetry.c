//
// Created by Aryan Dubey on 6/10/26.
//
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/myshell.h"

struct Memory {
    char *data;
    size_t size;
};



size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    struct Memory *mem = (struct Memory *)userdata;
    char *new_data = realloc(mem->data, mem->size + total + 1);
    if (!new_data) return 0;
    mem->data = new_data;
    memcpy(mem->data + mem->size, ptr, total);
    mem->size += total;
    mem->data[mem->size] = '\0';
    return total;
}

static size_t discard_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    return size * nmemb;
}

void send_telemetry(telemetry_t *t) {
    CURL *curl = curl_easy_init();
    if (!curl) return;
    char escaped_cmd[1024];
    int j = 0;
    for (int i = 0; t->command[i] && j < 1022; i++) {
        if (t->command[i] == '\\' || t->command[i] == '"') escaped_cmd[j++] = '\\';
        escaped_cmd[j++] = t->command[i];
    }
    escaped_cmd[j] = '\0';

    char json[2048];
    snprintf(json, sizeof(json),
        "{\"timestamp\": %ld, \"cwd\": \"%s\", \"cmd\": \"%s\", \"duration_ms\": %ld, \"exit\": %d}",
        t->timestamp, t->cwd, escaped_cmd, t->duration_ms, t->exit_code);

    struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, "127.0.0.1:8000/log");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void log_telemetry(telemetry_t *t) {
    getcwd(t->cwd, sizeof(t->cwd));
    t->timestamp = time(NULL);
    send_telemetry(t);
}

void get_prediction(char* cmd) {
    CURL *curl= curl_easy_init();
    if (!curl) {
        return;
    }
    struct Memory response;
    response.data = malloc(1);
    response.size = 0;
    response.data[0] = '\0';
    char json[1024];
    char escaped[1024];
    int j = 0;
    for (int i = 0; cmd[i] && j < 1022; i++) {
        if (cmd[i] == '\\') escaped[j++] = '\\';
        else if (cmd[i] == '"') escaped[j++] = '\\';
        escaped[j++] = cmd[i];
    }
    escaped[j] = '\0';
    snprintf(json, sizeof(json), "{\"cmd\": \"%s\", \"timestamp\": 0, \"exit\": 0, \"cwd\": \"/\", \"duration_ms\": 0}", escaped);

    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8000/predict");
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,json);
    struct curl_slist *headers = curl_slist_append(NULL, "Content-type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res= curl_easy_perform(curl);
    if (res != CURLE_OK) {
        free(response.data);
        fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
        return;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    cJSON *root = cJSON_Parse(response.data);
    if (!root) {
        fprintf(stderr, "Invalid JSON response\n");
        free(response.data);
        return;
    }
    cJSON *prediction = cJSON_GetObjectItem(root, "predicted_next_cmd");
    if (!cJSON_IsString(prediction)) {
        fprintf(stderr, "No prediction field\n");
        cJSON_Delete(root);
        free(response.data);
        return;
    }
    fprintf(stderr, "Predicted next command: %s\n", prediction->valuestring);
    cJSON_Delete(root);
    free(response.data);
}