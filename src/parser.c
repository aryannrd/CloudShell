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
char** parse(char* input) {
    char **args = malloc(64 * sizeof(char *));
    int argc=0;
    int is_token=0;
    int is_quotes=0;
    int is_path=0;
    size_t len= strlen(input);
    char pathvariable[1024];
    int path_count=0;
    for (int i=0; i<=len; i++) {
        if (input[i]=='\\' && input[i + 1] != '\0'){
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
            if (input[i]=='$') {
                path_count=0;
               int j=i+1;
                while (isalnum(input[j]) || input[j]=='_') {
                    pathvariable[path_count++] = input[j];
                    j++;
                }
                if (path_count == 0) {
                    args[argc] = &input[i];
                    argc++;
                    is_token = 1;
                }
                else {
                    pathvariable[path_count]='\0';
                    char* val= getenv(pathvariable);
                    if (val==NULL) {
                        val="";
                    }
                    is_token=0;
                    args[argc] = strdup(val);
                    argc++;
                    i=j-1;
                }
            }
            else {
                args[argc]=&input[i];
                argc++;
                is_token=1;
            }
        }
        else if (is_boundary && is_token==1) {
            if (is_quotes==1) {
                continue;
            }
            input[i] = '\0';
            is_token=0;
        }
    }
    args[argc] = NULL;
    return args;
}