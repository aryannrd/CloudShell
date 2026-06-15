#include <ctype.h>
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
job_t jobs[64];
int job_count = 0;

int main(void) {
    if (isatty(STDIN_FILENO)) {
        enable_raw();
        atexit(disable_raw);
    }
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, sigchild_handler);

    int last_status = 0;

    while (1) {
        if (sigchild == 1) {
            sigchild = 0;
            for (int i = 0; i < job_count; i++) {
                int status;
                pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
                if (result > 0 || result == -1) {
                    jobs[i].status = 1;
                    for (int j = i; j < job_count - 1; j++) {
                        jobs[j] = jobs[j + 1];
                    }
                    job_count--;
                    i--;
                }
            }
        }
        if (getcwd(interface, sizeof(interface)) == NULL) {
            strcpy(interface, "/");
        }
        read_line();
        if (buf[0] == '\0') {
            continue;
        }

        char cmd_parse[1024];
        char cmd_predict[1024];
        strncpy(cmd_parse, buf, sizeof(cmd_parse));
        cmd_parse[sizeof(cmd_parse) - 1] = '\0';
        strncpy(cmd_predict, buf, sizeof(cmd_predict));
        cmd_predict[sizeof(cmd_predict) - 1] = '\0';

        char **args = parse(cmd_parse);
        if (!args || !args[0]) {
            continue;
        }
        last_status = execute(args);
        get_prediction(cmd_predict);

        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
        if (last_status > 0) {
            printf("exit status: %d\n", last_status);
        }
    }
}