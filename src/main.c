#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "../include/myshell.h"
#include <sys/types.h>
#include <sys/wait.h>
#include "termios.h"
char buf[1024];
char interface[1024];

int main(void) {
    enable_raw();
    atexit(disable_raw);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, sigchild_handler);
    int last_status=0;

    while (1) {
        if (sigchild==1) {
            sigchild=0;
            for (int i = 0; i < job_count; i++) {
                int status;
                pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
                if (result > 0 || result==-1) {
                    jobs[i].status=1;
                    for (int j = i; j < job_count - 1; j++) {
                        jobs[j] = jobs[j+1];
                    }
                    job_count--;
                    i--;
                }
            }
        }
        if (getcwd(interface, sizeof(interface))!=NULL){
            printf("myshell:%s> ", interface);
            fflush(stdout);
        }
        else {
            printf("myshell> ");
            fflush(stdout);

        }
        read_line();
        printf("\n");
        fflush(stdout);
        if (strcmp(buf,"") == 0) {
            continue;
        }
        history[history_count] = strdup(buf);
        history_count++;
        char** args= parse(buf);
        if (args[0]==NULL) {
            continue;
        }
        last_status=execute(args);
        free(args);
        if (last_status!=0) {
            printf("exit status: %d\n", last_status);
        }
    }
}

