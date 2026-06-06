//
// Created by Aryan Dubey on 6/3/26.
//

#ifndef MOCK_SHELL_MYSHELL_H
#define MOCK_SHELL_MYSHELL_H
int execute(char** args);
char** parse(char*input);
int execute_builtin(char**args);
int check_builtin(char** args);
char** exec_output_redirect(char** args);
char** exec_input_redirect(char** args);
int has_redirect(char** args);
int exec_pipe(char** args, int count);
int has_pipe(char** args);
extern int bg_pids[64];
extern int bg_count;
#endif //MOCK_SHELL_MYSHELL_H
