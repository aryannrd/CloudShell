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

void send_telemetry(telemetry_t *t) {
    CURL *curl= curl_easy_init();
    if (!curl) {
        return;
    }
    char json[2048];
    snprintf(json,sizeof(json),"{\"timestamp\": %ld, \"cwd\": \"%s\", \"cmd\": \"%s\", \"duration_ms\": %ld, \"exit\": %d} \n ",t->timestamp,t->cwd, t->command, t->duration_ms, t->exit_code);

    curl_easy_setopt(curl, CURLOPT_URL, "http://host.docker.internal:8000/log");
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,json);
    struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
}

void log_telemetry(telemetry_t *t) {
    getcwd(t->cwd, sizeof(t->cwd));
    char path[1024];
    snprintf(path, sizeof(path), "%s/.myshell_history.jsonl", getenv("HOME"));

    FILE* fd= fopen(path,"a");
    if (fd == NULL) {
        perror(path);
        return;
    }
    t->timestamp=time(NULL);
    fprintf(fd, "{\"timestamp\": %ld, \"cwd\": \"%s\", \"cmd\": \"%s\", \"exit\": %d, \"duration_ms\": %ld}\n",
    t->timestamp, t->cwd, t->command, t->exit_code, t->duration_ms);
    fclose(fd);

    send_telemetry(t);
}
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    strncat((char*)userdata, ptr, size * nmemb);
    return size * nmemb;
}

void get_prediction(char* cmd) {
    CURL *curl= curl_easy_init();
    if (!curl) {
        return;
    }
    char json[1024];
    snprintf(json, sizeof(json), "{\"cmd\": \"%s\", \"timestamp\": 0, \"exit\": 0, \"cwd\": \"/\", \"duration_ms\": 0}", cmd);
    char response[4096]={0};
    curl_easy_setopt(curl, CURLOPT_URL, "http://host.docker.internal:8000/predict");
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,json);
    struct curl_slist *headers = curl_slist_append(NULL, "Content-type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    char *key = strstr(response, "predicted_next_cmd");

    int hit_semicolon=0;
    char* new_cmd= malloc(64*sizeof(char));
    if (key == NULL) {
        fprintf(stderr, "No prediction available\n");
        free(new_cmd);
        return;
    }
    int count_cmd=0;
    for (int i=0; key[i]!='\0';i++ ) {
        if (key[i]==',') {
            break;
        }
        if (key[i]==':') {
            hit_semicolon=1;
            continue;
        }
        if (hit_semicolon==1 && !isspace(key[i]) && key[i]!='\"') {
            new_cmd[count_cmd]=key[i];
            count_cmd++;
        }
    }
    new_cmd[count_cmd] = '\0';
    fprintf(stderr, "Predicted next command: %s\n", new_cmd);
    free(new_cmd);
}