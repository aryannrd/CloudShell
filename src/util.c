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
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

struct termios orig;
struct termios raw;

char *history[100];
int history_count = 0;

void enable_raw() {
    tcgetattr(STDIN_FILENO,&orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw);
    struct termios check;
    tcgetattr(STDIN_FILENO, &check);
}

void disable_raw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void read_line() {
    int counter = 0;
    int cursor = 0;
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
        if (c == 26) {
            counter = 0;
            cursor = 0;
            buf[0] = '\0';
            write(STDOUT_FILENO, "\nmyshell> ", 10);
            continue;
        }
        if (c == 127 || c == 8) {
            if (cursor > 0) {
                memmove(&buf[cursor-1], &buf[cursor], counter - cursor);
                counter--;
                cursor--;
                buf[counter] = '\0';
                write(STDOUT_FILENO, "\b", 1);
                write(STDOUT_FILENO, &buf[cursor], counter - cursor);
                write(STDOUT_FILENO, " ", 1);
                // move cursor back to correct position
                int move_back = counter - cursor + 1;
                for (int i = 0; i < move_back; i++)
                    write(STDOUT_FILENO, "\033[D", 3);
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
                // up arrow
                if (history_count > 0 && history_index > 0)
                    history_index--;
                if (history[history_index]) {
                    strcpy(buf, history[history_index]);
                    counter = strlen(buf);
                    cursor = counter;
                    write(STDOUT_FILENO, "\r\033[K", 4);
                    write(STDOUT_FILENO, "myshell:", 8);
                    write(STDOUT_FILENO, interface, strlen(interface));
                    write(STDOUT_FILENO, "> ", 2);
                    write(STDOUT_FILENO, buf, counter);
                }
            } else if (d == '[' && e == 'B') {
                // down arrow
                if (history_index < history_count - 1) {
                    history_index++;
                    strcpy(buf, history[history_index]);
                } else {
                    history_index = history_count;
                    buf[0] = '\0';
                }
                counter = strlen(buf);
                cursor = counter;
                write(STDOUT_FILENO, "\r\033[K", 4);
                write(STDOUT_FILENO, "myshell:", 8);
                write(STDOUT_FILENO, interface, strlen(interface));
                write(STDOUT_FILENO, "> ", 2);
                write(STDOUT_FILENO, buf, counter);
            } else if (d == '[' && e == 'C') {
                // right arrow
                if (cursor < counter) {
                    cursor++;
                    write(STDOUT_FILENO, "\033[C", 3);
                }
            } else if (d == '[' && e == 'D') {
                // left arrow
                if (cursor > 0) {
                    cursor--;
                    write(STDOUT_FILENO, "\033[D", 3);
                }
            }
            continue;
        }

        if (c >= 32 && c < 127) {
            memmove(&buf[cursor+1], &buf[cursor], counter - cursor);
            buf[cursor] = c;
            counter++;
            cursor++;
            buf[counter] = '\0';
            write(STDOUT_FILENO, &buf[cursor-1], counter - cursor + 1);
            int move_back = counter - cursor;
            for (int i = 0; i < move_back; i++)
                write(STDOUT_FILENO, "\033[D", 3);
        }
    }
}