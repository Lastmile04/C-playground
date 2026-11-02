#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int i = 1;
    bool omit_newLine = false;

    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        omit_newLine = true;
        i = 2;
    };

    for (; i < argc; i++) {
        fputs(argv[i], stdout);
        if (i < argc - 1)
            putchar(' ');
    };

    if (!omit_newLine)
        putchar('\n');

    return 0;
}
