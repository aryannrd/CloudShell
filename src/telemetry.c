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

void log_telemetry(telemetry_t *t) {
    getcwd(t->cwd, sizeof(t->cwd));
    char path[1024];
    snprintf(path, sizeof(path), "%s/.myshell_history.jsonl", getenv("HOME"));
    fprintf(stderr, "logging to: %s\n", path);
    FILE * fd= fopen(path,"a");
    if (fd == NULL) {
        perror(path);
        return;
    }
    t->timestamp=time(NULL);
    fprintf(fd,"{\"timestamp\": %ld, \"cwd\": \"%s\", \"cmd\": \"%s\", \"duration_ms\": %ld \"exit\": %d} \n ",t->timestamp,t->cwd, t->command, t->duration_ms, t->exit_code);
    fclose(fd);
}