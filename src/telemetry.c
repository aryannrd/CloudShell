//
// Created by Aryan Dubey on 6/10/26.
//
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
    fprintf(stderr, "logging to: %s\n", path);

    FILE* fd= fopen(path,"a");
    if (fd == NULL) {
        perror(path);
        return;
    }
    t->timestamp=time(NULL);
    fprintf(fd,"{\"timestamp\": %ld, \"cwd\": \"%s\", \"cmd\": \"%s\", \"duration_ms\": %ld, \"exit\": %d} \n ",t->timestamp,t->cwd, t->command, t->duration_ms, t->exit_code);
    fclose(fd);

    send_telemetry(t);
}