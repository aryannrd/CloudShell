//
// Created by Aryan Dubey on 6/3/26.
//
#include <signal.h>

#include "../include/myshell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int execute_builtin(char**args) {
    if (strcmp(args[0],"fg")==0){
        if (args[1] == NULL) {
            fprintf(stderr, "fg: no job specified\n");
            return -1;
        }
        int job_idx = atoi(args[1]) - 1;
        if (job_idx < 0 || job_idx >= job_count) {
            fprintf(stderr, "fg: job %s not found\n", args[1]);
            return -1;
        }
        tcsetpgrp(STDIN_FILENO,jobs[job_idx].pgid);
        kill(-jobs[job_idx].pgid, SIGCONT);
        disable_raw();
        int status;
        waitpid(jobs[job_idx].pid,&status,WUNTRACED);
        tcsetpgrp(STDIN_FILENO,getpgrp());
        enable_raw();
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            free(jobs[job_idx].command);
            for (int i=job_idx; i<job_count-1;i++) {
                jobs[i]=jobs[i+1];
            }
            job_count--;
        }
        else if (WIFSTOPPED(status)) {
            jobs[job_idx].stopped=1;
        }
        return 0;
    }
    if (strcmp(args[0],"bg")==0) {
        if (args[1] == NULL) {
            fprintf(stderr, "bg: no job specified\n");
            return -1;
        }
        int job_idx = atoi(args[1]) - 1;
        if (job_idx < 0 || job_idx >= job_count) {
            fprintf(stderr, "bg: job %s not found\n", args[1]);
            return -1;
        }
        jobs[job_idx].stopped = 0;
        kill(-jobs[job_idx].pgid, SIGCONT);
        return 0;
    }
    char* target_path = args[1];
    if (target_path == NULL) {
        target_path = getenv("HOME");
    }
    if (strcmp(args[0],"cd")==0) {
        if (chdir(target_path)==-1) {
            fprintf(stderr,"%s was not found\n", target_path);
            return -1;
        }
        return 0;
    }
    if (strcmp(args[0],"exit")==0) {
        disable_raw();
        exit(0);
    }
    if (strcmp(args[0],"jobs")==0) {
        for (int i=0; i<job_count;i++) {
            printf("[%d] %d %s  %s \n",i+1, jobs[i].pid, jobs[i].stopped == 1 ? "stopped" : "running", jobs[i].command);
        }
        return 0;
    }
    return -1;
}

int check_builtin(char** args) {
    if (strcmp(args[0],"cd")==0) {
        return 1;
    }
    if (strcmp(args[0],"exit")==0) {
        return 1;
    }
    if (strcmp(args[0],"jobs")==0){
        return 1;
    }
    if (strcmp(args[0],"fg")==0){
        return 1;
    }
    if (strcmp(args[0],"bg")==0){
        return 1;
    }
    return 0;
}
