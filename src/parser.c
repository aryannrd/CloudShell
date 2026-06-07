//
// Created by Aryan Dubey on 6/3/26.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char** parse(char* input) {
    char **args = malloc(64 * sizeof(char *));
    int argc=0;
    int is_token=0;
    int is_quotes=0;
    size_t len= strlen(input);

    for (int i=0; i<=len; i++) {
        if (input[i]=='\\'){
            input[i]='\0';
            i++;
        }
        int is_boundary = (input[i] == '\0' || isspace((unsigned char)input[i]));
        if (input[i] == '"') {
            input[i] = '\0';
            if (is_quotes == 0) {
                is_quotes = 1;
            } else {
                is_quotes = 0;
                is_token = 0;
            }
            continue;
        }
        if (!is_boundary && is_token==0){
            args[argc]=&input[i];
            argc++;
            is_token=1;
        }
        else if (is_boundary && is_token==1) {
            if (is_quotes==1) {
                continue;
            }
            input[i] = '\0';
            is_token=0;
        }
    }
    args[argc]=NULL;
    return args;
}