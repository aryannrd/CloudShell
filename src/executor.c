#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "../include/myshell.h"
#include <sys/types.h>

int has_background(char **args) {
    for (int i=0; args[i]!=NULL; i++) {
        if (args[i][0]=='&') {
            args[i]=NULL;
            return 1;
        }
    }
    return 0;
}

int execute(char** args) {
    char new_buf[1024];
    struct timespec start, end;
    new_buf[0]='\0';
    for (int i=0;args[i]!=NULL;i++) {
        if (args[i][0]=='&') {
            continue;
        }
        strcat(new_buf,args[i]);
        if (args[i+1]!=NULL && args[i+1][0]!='&') {
            strcat(new_buf," ");
        }
    }
    if (has_background(args)==1) {
        clock_gettime(CLOCK_MONOTONIC,&start);
        pid_t p = fork();
        if (p==-1) {
            return -1;
        }
        if (p==0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            setpgid(0,0);
            execvp(args[0],args);
            fprintf(stderr,"%s command not found.", args[0]);
            free(args);
            exit(127);
        }
        else {
            setpgid(p, p);
            jobs[job_count].pid = p;
            jobs[job_count].pgid = p;
            jobs[job_count].command = strdup(new_buf);
            jobs[job_count].status = 0;
            jobs[job_count].stopped = 0;
            job_count++;
            return 0;
        }
    }
    if (check_builtin(args)==1) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int builtin_status = execute_builtin(args);
        clock_gettime(CLOCK_MONOTONIC, &end);
        telemetry_t t;
        long duration_ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
        strcpy(t.command, new_buf);
        t.exit_code = builtin_status;
        t.duration_ms = duration_ms;
        log_telemetry(&t);
        return builtin_status;
    }
    int pipe_count = has_pipe(args);
    if (pipe_count >= 1) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        int pipe_status=exec_pipe(args, pipe_count);
        clock_gettime(CLOCK_MONOTONIC, &end);
        telemetry_t t;
        long duration_ms = (end.tv_sec - start.tv_sec) * 1000 +
                           (end.tv_nsec - start.tv_nsec) / 1000000;
        strcpy(t.command, new_buf);
        t.exit_code = pipe_status;
        t.duration_ms = duration_ms;
        log_telemetry(&t);
        return pipe_status;
    }
    clock_gettime(CLOCK_MONOTONIC,&start);
    pid_t p = fork();
    if (p==-1) {
        return -1;
    }
    if (p==0) {
        int redir = has_redirect(args);
        if (redir==1) {
            char** clean_args=exec_output_redirect(args);
            if (clean_args == NULL) { exit(1); }
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            setpgid(0,0);
            execvp(clean_args[0], clean_args);
            free(clean_args);
            fprintf(stderr, "%s command not found\n",args[0]);
            exit(127);
        }
        else if (redir==2) {
            char** clean_args=exec_input_redirect(args);
            if (clean_args == NULL) { exit(1); }
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            setpgid(0,0);
            execvp(clean_args[0], clean_args);
            free(clean_args);
            fprintf(stderr, "%s command not found\n",args[0]);
            exit(127);
        }
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        setpgid(0,0);
        execvp(args[0],args);
        fprintf(stderr, "%s command not found\n",args[0]);
        exit(127);
    }
    else {
        setpgid(p, p);
        tcsetpgrp(STDIN_FILENO, p);
        int status;
        jobs[job_count].pid = p;
        jobs[job_count].pgid = p;
        jobs[job_count].command = strdup(new_buf);
        jobs[job_count].status = 0;
        jobs[job_count].stopped = 0;
        job_count++;
        waitpid(p,&status,WUNTRACED);
        clock_gettime(CLOCK_MONOTONIC, &end);
        tcsetpgrp(STDIN_FILENO, getpgrp());
        if (WIFSTOPPED(status)) {
            jobs[job_count-1].stopped=1;
        }
        else {
            job_count--;
            free(jobs[job_count].command);
            jobs[job_count].pid = 0;
        }
        telemetry_t t;
        long duration_ms=(end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
        strcpy(t.command,new_buf);
        t.exit_code= WEXITSTATUS(status);
        t.duration_ms = duration_ms;
        log_telemetry(&t);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }
    return -1;
}