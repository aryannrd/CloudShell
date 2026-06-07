//
// Created by Aryan Dubey on 6/3/26.
//
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include "../include/myshell.h"

void sigchild_handler(int sig) {
    waitpid(-1,NULL, WNOHANG);
}
