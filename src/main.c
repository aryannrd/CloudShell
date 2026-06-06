#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/myshell.h"

int main(void) {
    for (int i = 0; i < bg_count; i++) {
        int status;
        pid_t result = waitpid(bg_pids[i], &status, WNOHANG);
        if (result > 0) {
            for (int j = i; j < bg_count - 1; j++) {
                bg_pids[j] = bg_pids[j+1];
            }
            bg_count--;
            i--;
        }
    }
    char buf[1024];
    char interface[1024];
    int last_status=0;
    while (1) {
        if (getcwd(interface, sizeof(interface))!=NULL){
            printf("myshell:%s> ", interface);
            fflush(stdout);
        }
        else {
            printf("myshell> ");
            fflush(stdout);

        }
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        buf[strcspn(buf, "\n")]='\0';
        if (strcmp(buf,"") == 0) {
            continue;
        }
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

