//
// Created by Aryan Dubey on 6/3/26.
#include <stdio.h>
#include <stdlib.h>
    #include "../include/myshell.h"
#include <unistd.h>
#include <_string.h>

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
    new_buf[0]='\0';
    for (int i=0;args[i]!=NULL;i++) {
        if (args[i+1]!=NULL) {
            strcat(new_buf," ");
        }
        if (args[i][0]=='&') {
            continue;
        }
        strcat(new_buf,args[i]);
    }
    if (has_background(args)==1) {
        pid_t p = fork();
        if (p==-1) {
            return -1;
        }
        if (p==0) {
            execvp(args[0],args);
            fprintf(stderr,"%s command not found.", args[0]);
            free(args);
            exit(127);
        }
        else {
            jobs[job_count].pid = p;
            jobs[job_count].command = strdup(new_buf);
            job_count++;
            return 0;
        }
    }
    if (check_builtin(args)==1) {
        return execute_builtin(args);
    }
    int pipe_count = has_pipe(args);
    if (pipe_count >= 1) {
        return exec_pipe(args, pipe_count);
    }
    pid_t p = fork();
    if (p==-1) {
        return -1;
    }
    if (p==0) {
        int redir = has_redirect(args);
        if (redir==1) {
            char** clean_args=exec_output_redirect(args);
            if (clean_args == NULL) { exit(1); }
            execvp(clean_args[0], clean_args);
            free(clean_args);
            fprintf(stderr, "%s command not found\n",args[0]);
            exit(127);
        }
        else if (redir==2) {
            char** clean_args=exec_input_redirect(args);
            if (clean_args == NULL) { exit(1); }
            execvp(clean_args[0], clean_args);
            free(clean_args);
            fprintf(stderr, "%s command not found\n",args[0]);
            exit(127);
        }
        execvp(args[0],args);
        fprintf(stderr, "%s command not found\n",args[0]);
        exit(127);
    }
    else {
        int status;
        waitpid(p,&status,0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }
    return -1;
}


