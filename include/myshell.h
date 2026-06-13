//
// Created by Aryan Dubey on 6/3/26.
//

#ifndef MOCK_SHELL_MYSHELL_H
#define MOCK_SHELL_MYSHELL_H
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

extern char interface[1024];
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
    pid_t pgid;
    char *command;
    int status;
    int stopped;
} job_t;

extern job_t jobs[64];
extern int job_count;

void sigchild_handler(int sig);
extern volatile sig_atomic_t sigchild;

extern struct termios raw;
void enable_raw();
void disable_raw();
void read_line();

extern char *history[100];
extern int history_count;

typedef struct {
    time_t timestamp;
    char cwd[1024];
    char command[1024];
    long duration_ms;
    int exit_code;
} telemetry_t;

void log_telemetry(telemetry_t *t);
void get_prediction(char* cmd);
#endif //MOCK_SHELL_MYSHELL_H
