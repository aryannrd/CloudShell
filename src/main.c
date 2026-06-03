#include <stdio.h>
#include <string.h>


int main(void) {
    char buf[1024];
    while (1) {
        printf("myshell> ");
        char* input = fgets(buf, sizeof(buf),stdin);
        if (input==NULL) {
            break;
        }
        input[strcspn(input, "\n")]='\0';
        if (strcmp(input,"") == 0) {
            continue;
        }
        printf("Got %s\n", input);
    }
}

