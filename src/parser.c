//
// Created by Aryan Dubey on 6/3/26.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char** parser(char* input) {
    char **args = malloc(64 * sizeof(char *));
    int argc=0;
    int is_token=0;
    for (int i=0; i<strlen(input); i++) {
        if (isspace(input[i])==0 && is_token==0){
            args[argc]=&input[i];
            argc++;
            is_token=1;
        }
        else if (isspace(input[i])!=0 && is_token==1) {
            input[i] = '\0';
            is_token=0;
        }
    }
    args[argc]=NULL;
    return args;
}