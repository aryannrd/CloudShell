#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/myshell.h"

int main(void) {
    char buf[1024];
    char interface[1024];

    while (1) {
        if (getcwd(interface, sizeof(interface))!=NULL){
            printf("myshell:%s> ", interface);
        }
        else {
            printf("myshell> ");
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
        execute(args);
        free(args);
    }
}

