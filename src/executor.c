//
// Created by Aryan Dubey on 6/3/26.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int execute(char** args) {
    pid_t p = fork();
    if (p==-1) {
        return -1;
    }
    if (p==0) {
        execvp(args[0],args);
        fprintf(stderr, "%s command not found",args[0]);
        exit(1);
    }
    else {
        int status;
        waitpid(p,&status,0);
        return WEXITSTATUS(status);
    }

}