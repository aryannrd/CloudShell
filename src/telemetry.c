//
// Created by Aryan Dubey on 6/10/26.
//
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include "../include/myshell.h"
#include <sys/types.h>
#include <curl/curl.h>
#include <time.h>

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

void send_telemetry(telemetry_t *t) {
    CURL *curl= curl_easy_init();
    if (!curl) {
        return;
    }

    char json[2048];
    snprintf(json,sizeof(json),"{\"timestamp\": %ld, \"cwd\": \"%s\", \"cmd\": \"%s\", \"duration_ms\": %ld, \"exit\": %d}",t->timestamp,t->cwd, t->command, t->duration_ms, t->exit_code);

    char response[1024] = {0};

    curl_easy_setopt(curl, CURLOPT_URL, "http://host.docker.internal:8000/log");
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,json);

    struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
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
    snprintf(json, sizeof(json), "{\"cmd\": \"%s\", \"timestamp\": 0, \"exit\": 0, \"cwd\": \"/\", \"duration_ms\": 0}", cmd);
    curl_easy_setopt(curl, CURLOPT_URL, "http://host.docker.internal:8000/predict");
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,json);
    struct curl_slist *headers = curl_slist_append(NULL, "Content-type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    char *key = strstr(response.data, "predicted_next_cmd");
    if (key == NULL) {
        fprintf(stderr, "No prediction available\n");
        free(response.data);
        return;
    }
    char *start = strchr(key, ':');
    if (!start) {
        fprintf(stderr, "No prediction available\n");
        free(response.data);
        return;
    }
    start++;
    while (*start == ' ' || *start == '\"') start++;
    char new_cmd[128];
    int i = 0;
    while (*start && *start != '\"' && *start != ',' && i < 127) {
        new_cmd[i++] = *start++;
    }
    new_cmd[i] = '\0';
    fprintf(stderr, "Predicted next command: %s\n", new_cmd);
    free(response.data);
}