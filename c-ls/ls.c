#include <stdio.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    const char *path = (argc>1)? argv[1] :  ".";

    DIR *dir = opendir(path);
    // printf("%s\n");
    return 0;
}
