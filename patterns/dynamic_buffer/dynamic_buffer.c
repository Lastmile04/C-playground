#include <stdio.h>
#include <stdlib.h>

char *readLine(FILE *fp){

    int offset = 0;
    int bufSize = 4;
    char *buf;
    int c;

    buf = malloc(bufSize);

    if(buf == NULL) return NULL;

    // expand the buffer dynamically 
    while(c = fgetc(fp), c != '\n' && c != EOF){

        if(offset == bufSize -1){
            bufSize *=2;

            char *new_buf = realloc(buf, bufSize);

            if(new_buf == NULL){
                free(new_buf);
                return NULL;
            }
            buf = new_buf;
        }
        buf[offset++] = c;
    }

    if( c == EOF && offset == 0){
        free(buf);
        return NULL;
    }

    //shrink the buffer 
    if(offset < bufSize-1){
        char *new_buf = realloc(buf,bufSize-1);

        if(new_buf != NULL) buf = new_buf;
    }
    buf[offset] = '\0';
    return buf;
}

int main(void){
    FILE *fp = fopen("file.txt", "r");

    char *line;
    while ((line = readLine(fp)) != NULL) {
        printf("%s\n", line);
        free(line);
    }
    fclose(fp);
}

