//
// Created by Aryan Dubey on 6/3/26.
//
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

char** exec_output_redirect(char** args) {
    char **new_args = malloc(64 * sizeof(char *));
    int argc = 0;

    for (int i = 0; args[i] != NULL;) {
        if (args[i][0] == '>') {
            char* file_name = NULL;
            if (args[i][1] != '\0') {
                file_name = &args[i][1];
                i += 1;
            } else {
                file_name = args[i + 1];
                if (file_name == NULL) {
                    fprintf(stderr, "Syntax error: expected file after '>'\n");
                    free(new_args);
                    return NULL;
                }
                i += 2;
            }
            int fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if (fd == -1) {
                perror(file_name);
                free(new_args);
                return NULL;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        else {
            new_args[argc] = args[i];
            argc++;
            i++;
        }
    }
    new_args[argc] = NULL;
    return new_args;
}

char ** exec_input_redirect(char** args) {
    char **new_args = malloc(64 * sizeof(char *));
    int argc = 0;

    for (int i = 0; args[i] != NULL;) {
        if (args[i][0] == '<') {
            char* file_name = NULL;
            if (args[i][1] != '\0') {
                file_name = &args[i][1];
                i += 1;
            } else {
                file_name = args[i + 1];
                if (file_name == NULL) {
                    fprintf(stderr, "Syntax error: expected file after '<'\n");
                    free(new_args);
                    return NULL;
                }
                i += 2;
            }
            int fd = open(file_name, O_RDONLY);
            if (fd == -1) {
                perror(file_name);
                free(new_args);
                return NULL;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        else {
            new_args[argc] = args[i];
            argc++;
            i++;
        }
    }
    new_args[argc] = NULL;
    return new_args;
}



int has_redirect(char** args) {
    for (int i=0; args[i]!=NULL;i++) {
        if (args[i][0]=='>') {
            return 1;
        }
        else if (args[i][0]=='<') {
            return 2;
        }
    }

    return 0;
}