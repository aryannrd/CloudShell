//
// Created by Aryan Dubey on 6/3/26.
#include <stdio.h>
#include <stdlib.h>
    #include "../include/myshell.h"
#include <unistd.h>
int execute(char** args) {
    if (check_builtin(args)==1) {
        return execute_builtin(args);
    }
    pid_t p = fork();
    if (p==-1) {
        return -1;
    }
    if (p==0) {
        if (has_redirect(args)==1) {
            char** clean_args=exec_redirect(args);
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