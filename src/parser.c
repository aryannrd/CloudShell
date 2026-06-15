//
// Created by Aryan Dubey on 6/3/26.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
char * com;
char **parse(char *input) {
    char **args = malloc(64 * sizeof(char *));
    int argc = 0;
    char *token_start = NULL;
    int is_quotes = 0;
    size_t len = strlen(input);
    char pathvariable[1024];
    int path_count = 0;

    for (int i = 0; i <= (int)len; i++) {
        if (input[i] == '\\' && input[i + 1] != '\0') {
            input[i] = '\0';
            i++;
        }
        int is_boundary = (input[i] == '\0' || (isspace((unsigned char)input[i]) && !is_quotes));

        if (input[i] == '"') {
            input[i] = '\0';
            if (!is_quotes) {
                is_quotes = 1;
                if (!token_start) token_start = &input[i + 1];
            } else {
                is_quotes = 0;
                // end of quoted token — will be picked up at next boundary
            }
            continue;
        }

        if (!is_boundary && !token_start) {
            if (input[i] == '$') {
                path_count = 0;
                int j = i + 1;
                while (isalnum(input[j]) || input[j] == '_')
                    pathvariable[path_count++] = input[j++];
                if (path_count == 0) {
                    token_start = &input[i];
                } else {
                    pathvariable[path_count] = '\0';
                    char *val = getenv(pathvariable);
                    args[argc++] = strdup(val ? val : "");
                    i = j - 1;
                }
            } else {
                token_start = &input[i];
            }
        } else if (is_boundary && token_start) {
            input[i] = '\0';
            args[argc++] = strdup(token_start);
            token_start = NULL;
        }
    }
    args[argc] = NULL;
    return args;
}