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
#include "../include/myshell.h"

int exec_pipe(char** args, int count) {
    int pipe_count=count;
    int cmd_count=pipe_count+1;
    int start=0;
    int index=0;
    char*** commands= malloc(cmd_count*sizeof(char**));

    for (int i=0; ;i++) {
        if (args[i] == NULL || strcmp(args[i], "|") == 0) {
            int len = i - start;
            commands[index] = malloc((len + 1) * sizeof(char*));

            for (int j=0; j<len; j++) {
                commands[index][j]=args[start+j];
            }
            commands[index][len]=NULL;
            index++;
            start=i+1;
            if (args[i]==NULL) {
                break;
            }
        }
    }
    pid_t pids[cmd_count];
    int prev_fd=-1;

    for (int i=0; i<cmd_count;i++) {
        int fd[2];
        if (i < cmd_count - 1) {
            if (pipe(fd) == -1) {
                perror("pipe");
                return -1;
            }
        }

        pid_t p=fork();
        if (p==-1) {
            perror("fork");
            return -1;
        }
        if (p==0) {
            setpgid(0,0);
            if (prev_fd!=-1) {
                dup2(prev_fd,STDIN_FILENO);
            }
            if (i<cmd_count-1) {
                dup2(fd[1], STDOUT_FILENO);

            }
            if (prev_fd!=-1) {
                close(prev_fd);
            }
            if (i<cmd_count-1) {
                close(fd[0]);
                close(fd[1]);
            }
            int redirect_condition= has_redirect(commands[i]);
            if (i == cmd_count - 1 && redirect_condition) {
                if (redirect_condition==1) {
                    char** clean_args=exec_output_redirect(commands[i]);
                    if (clean_args == NULL) { exit(1); }
                    signal(SIGINT,SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    setpgid(0, 0);
                    execvp(clean_args[0], clean_args);
                    free(clean_args);
                    fprintf(stderr, "%s command not found\n",args[0]);
                    exit(127);
                }
                else if (redirect_condition==2) {
                    char** clean_args=exec_input_redirect(commands[i]);
                    if (clean_args == NULL) { exit(1); }
                    signal(SIGINT,SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    setpgid(0, 0);
                    execvp(clean_args[0], clean_args);
                    free(clean_args);
                    fprintf(stderr, "%s command not found\n",args[0]);
                    exit(127);
                }
            }
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            setpgid(0, 0);
            execvp(commands[i][0],commands[i]);
            fprintf(stderr, "%s command not found\n",commands[i][0]);
            free(commands[i]);
            exit(127);

        }
        else {
            pids[i]=p;
            if (i == 0) {
                setpgid(p, p);
            } else {
                setpgid(p, pids[0]);
            }
            if (prev_fd!=-1) {
                close(prev_fd);
            }
            if (i < cmd_count - 1) {
                close(fd[1]);
                prev_fd = fd[0];
            }
        }
    }
    int last_status;
    for (int i = 0; i < cmd_count; i++) {
        int status;
        waitpid(pids[i], &status, WUNTRACED);
        if (i == cmd_count - 1) {
            last_status = status;
        }
        if (WIFSTOPPED(status)) {
            if (i== cmd_count-1) {
                jobs[job_count].pid= pids[0];
                jobs[job_count].pgid = pids[0];
                jobs[job_count].command = strdup(buf);
                jobs[job_count].stopped = 1;
                jobs[job_count].status = 0;
                job_count++;
                tcsetpgrp(STDIN_FILENO, getpgrp());
            }
        }
    }

    for (int i = 0; i < cmd_count; i++) {
        free(commands[i]);
    }
    free(commands);
    if (WIFEXITED(last_status)) {
        return WEXITSTATUS(last_status);
    }
    return -1;
}



int has_pipe(char** args) {
    int count=0;
    for (int i=0; args[i]!=NULL;i++) {
        if (args[i][0]=='|') {
            count+=1;
        }
    }
    if (count>=1) {
        return count;
    }
    return 0;
}
