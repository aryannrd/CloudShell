//
// Created by Aryan Dubey on 6/3/26.
//

#ifndef MOCK_SHELL_MYSHELL_H
#define MOCK_SHELL_MYSHELL_H
int execute(char** args);
char** parse(char*input);
int execute_builtin(char**args);
int check_builtin(char** args);
#endif //MOCK_SHELL_MYSHELL_H
