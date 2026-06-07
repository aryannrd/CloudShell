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
#include <termios.h>

struct termios orig;
struct termios raw;

char *history[100];
int history_count = 0;

void enable_raw() {
    fprintf(stderr, "enabling raw mode\n");
    tcgetattr(STDIN_FILENO,&orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw);
    struct termios check;
    tcgetattr(STDIN_FILENO, &check);
    fprintf(stderr, "ECHO set: %d\n", (check.c_lflag & ECHO) != 0);
    fprintf(stderr, "ICANON set: %d\n", (check.c_lflag & ICANON) != 0);
}

void disable_raw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void read_line() {
    int counter = 0;
    char c;
    int history_index = history_count;
    buf[0] = '\0';
    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1)
            continue;
        if (c == '\n' || c == '\r') {
            write(STDOUT_FILENO, "\n", 1);
            buf[counter] = '\0';
            return;
        }
        if (c == 127 || c == 8) {
            if (counter > 0) {
                counter--;
                buf[counter] = '\0';
                write(STDOUT_FILENO, "\b \b", 3);
            }
            continue;
        }

        if (c == 27) {
            char d, e;
            if (read(STDIN_FILENO, &d, 1) != 1)
                continue;
            if (read(STDIN_FILENO, &e, 1) != 1)
                continue;
            if (d == '[' && e == 'A') {
                if (history_count > 0 && history_index > 0)
                    history_index--;

                if (history[history_index]) {
                    strcpy(buf, history[history_index]);
                    counter = strlen(buf);

                    write(STDOUT_FILENO, "\r\033[K", 4);
                    write(STDOUT_FILENO, "myshell> ", 9);
                    write(STDOUT_FILENO, interface, strlen(interface));
                    write(STDOUT_FILENO, "> ", 2);
                    write(STDOUT_FILENO, buf, counter);
                }
            } else if (d == '[' && e == 'B') {
                if (history_index < history_count - 1) {
                    history_index++;
                    strcpy(buf, history[history_index]);
                } else {
                    history_index = history_count;
                    buf[0] = '\0';
                }

                counter = strlen(buf);

                write(STDOUT_FILENO, "\r\033[K", 4);
                write(STDOUT_FILENO, "myshell> ", 9);
                write(STDOUT_FILENO, interface, strlen(interface));
                write(STDOUT_FILENO, "> ", 2);
                write(STDOUT_FILENO, buf, counter);
            }

            continue;
        }

        if (c >= 32 && c < 127) {
            buf[counter++] = c;
            buf[counter] = '\0';
            write(STDOUT_FILENO, &c, 1);
        }
    }
}