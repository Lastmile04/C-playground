#include <errno.h>
#include <fcntl.h> // For open(), O_RDONLY, O_NOFOLLOW
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h> // For close()
                    //
void handle_write(FILE *fp) {
    char buf[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
        size_t bytesWritten = fwrite(buf, sizeof(char), bytesRead, stdout);

        if (bytesWritten < bytesRead) {
            fprintf(stderr, "ccat: write error: %s\n", strerror(errno));
            exit(1);
        }
    }

    if (ferror(fp)) {
        perror("ccat: read error");
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
        // Open the file first with O_NOFOLLOW to prevent following symlinks automatically
        int fd = open(argv[f], O_RDONLY | O_NOFOLLOW);
        if (fd < 0) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "%s", strerror(errno));
            error_count++;
            continue;
        }

        // Perform fstat on the active file descriptor (completely race-free)
        if (fstat(fd, &st) < 0) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "%s", strerror(errno));
            error_count++;
            close(fd);
            continue;
        }

        // Check if it's a directory
        if (S_ISDIR(st.st_mode)) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Is a directory");
            error_count++;
            close(fd);
            continue;
        }

        // Check if it's a symbolic link (O_NOFOLLOW with open() will actually cause open to fail with ELOOP
        if (S_ISLNK(st.st_mode)) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Is a symbolic link");
            error_count++;
            close(fd);
            continue;
        }

        // Convert the raw file descriptor into a FILE * stream using fdopen
        fp = fdopen(fd, "rb");
        if (fp == NULL) {
            errors[error_count].filename = argv[f];
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "%s", strerror(errno));
            error_count++;
            close(fd); // Close raw fd if fdopen failed
            continue;
        }

        handle_write(fp);
        fclose(fp); // fclose automatically closes the underlying fd as well
    }

    for (int i = 0; i < error_count; i++) {
        fprintf(stderr, "ccat: %s: %s\n", errors[i].filename, errors[i].message);
    }

    return error_count > 0 ? 1 : 0;
}
