#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void handle_write(FILE *fp) {

    char buf[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
        fwrite(buf, sizeof(char), bytesRead, stdout);
    }
}

struct DeferredError {
    const char *filename;
    char message[256];
};

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc == 1) {
        handle_write(stdin);
        return 0;
    }

    struct stat st;
    struct DeferredError errors[argc];
    int error_count = 0;

    for (int f = 1; f < argc; f++) {
        if (stat(argv[f], &st) < 0) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "%s", strerror(errno));
            error_count++;
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "%s", "Is a directory");
            error_count++;
            continue;
        }

        if (S_ISLNK(st.st_mode)) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "%s", "Is a symbolic link");
            error_count++;
            continue;
        }

        fp = fopen(argv[f], "rb");
        if (fp == NULL) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "%s", strerror(errno));
            error_count++;
            continue;
        }
        handle_write(fp);
        fclose(fp);
    }

    for (int i = 0; i < error_count; i++) {
        fprintf(stderr, "ccat: %s: %s\n", errors[i].filename, errors[i].message);
    }

    return error_count > 0 ? 1 : 0;
}
