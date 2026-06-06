//
// Created by Aryan Dubey on 6/3/26.
//
#include "../include/myshell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int execute_builtin(char**args) {
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
        exit(0);
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
    return 0;
}


