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

int exec_pipe(char** args) {
    int breakpoint=0;
    char **left_args = malloc(64 * sizeof(char *));
    int argl=0;
    int argr=0;
    char **right_args = malloc(64 * sizeof(char *));
    for (int i=0; args[i]!=NULL;) {
        if (args[i][0]!='|') {
            left_args[argl]=args[i];
            argl+=1;
            i++;
        }
        else {
            breakpoint = i;
            break;
        }
    }
    left_args[argl] = NULL;
    for (int i=(breakpoint+1);args[i]!=NULL;) {
        right_args[argr]=args[i];
        argr++;
        i++;
    }
    right_args[argr] = NULL;
    int fd[2];
    pipe(fd);
    pid_t left_fork=fork();
    if (left_fork==-1) {
        return -1;
    }
    if (left_fork==0) {
        dup2(fd[1],STDOUT_FILENO);
        close(fd[0]);
        execvp(left_args[0], left_args);
        fprintf(stderr, "%s command not found\n",left_args[0]);
        free(left_args);
        exit(127);
    }
    else {
        pid_t right_fork=fork();
        if (right_fork==-1) {
            return -1;
        }
        if (right_fork==0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            execvp(right_args[0], right_args);
            fprintf(stderr, "%s command not found\n",right_args[0]);
            free(right_args);
            exit(127);
        }
        else {
            close(fd[0]);
            close(fd[1]);
            int left_status;
            int right_status;
            waitpid(left_fork,&left_status,0);
            waitpid(right_fork,&right_status,0);
            if (WIFEXITED(left_status)&& WIFEXITED(right_status)) {
                return WEXITSTATUS(right_status);
            }
        }
    }
    return -1;
}

int has_pipe(char** args) {
    for (int i=0; args[i]!=NULL;i++) {
        if (args[i][0]=='|') {
            return 1;
        }
    }
    return 0;
}
