#include <stdio.h>
#include <dirent.h>

int main(int argc, char *argv[]) 
{
    const char *path = (argc>1)? argv[1] :  ".";

    DIR *dir = opendir(path);
    if(!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){
        if(entry ->d_name[0] == '.') continue; // to skip the dot files  
        printf("%s\n", entry->d_name);  // d_name comes from the dereferencing the struct: (entry*).d_name
    }   
    closedir(dir);
    return 0;
}
