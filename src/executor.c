#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "../include/myshell.h"

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
        if (args[i][0]=='&') {
            continue;
        }
        strcat(new_buf,args[i]);
        if (args[i+1]!=NULL && args[i+1][0]!='&') {
            strcat(new_buf," ");
        }
    }
    if (has_background(args)==1) {
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
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            setpgid(0,0);
            tcsetpgrp(STDIN_FILENO, getpid());
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
            tcsetpgrp(STDIN_FILENO, getpid());
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
        tcsetpgrp(STDIN_FILENO, getpid());
        execvp(args[0],args);
        fprintf(stderr, "%s command not found\n",args[0]);
        exit(127);
    }
    else {
        setpgid(p, p);
        tcsetpgrp(STDIN_FILENO, p);
        disable_raw();
        int status;
        jobs[job_count].pid = p;
        jobs[job_count].pgid = p;
        jobs[job_count].command = strdup(new_buf);
        jobs[job_count].status = 0;
        jobs[job_count].stopped = 0;
        job_count++;
        waitpid(p,&status,WUNTRACED);
        tcsetpgrp(STDIN_FILENO, getpgrp());
        enable_raw();
        if (WIFSTOPPED(status)) {
            jobs[job_count-1].stopped=1;
        }
        else {
            job_count--;
            free(jobs[job_count].command);
            jobs[job_count].pid = 0;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }
    return -1;
}