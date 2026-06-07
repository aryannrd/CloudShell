//
// Created by Aryan Dubey on 6/3/26.
//

#ifndef MOCK_SHELL_MYSHELL_H
#define MOCK_SHELL_MYSHELL_H
#include <stdlib.h>
int execute(char** args);
char** parse(char*input);
int execute_builtin(char**args);
int check_builtin(char** args);
char** exec_output_redirect(char** args);
char** exec_input_redirect(char** args);
int has_redirect(char** args);
int exec_pipe(char** args, int count);
int has_pipe(char** args);
extern char buf[1024];
typedef struct {
    pid_t pid;
    char *command;
    int status;
} job_t;

extern job_t jobs[64];
extern int job_count;

void sigchild_handler(int sig);
#endif //MOCK_SHELL_MYSHELL_H
