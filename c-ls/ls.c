#include <stdio.h>
#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>

// global bool var for differnt option flags
int show_all = 0;       // show hidden files
int long_format = 0;    // show long format (sys stat info)

int main(int argc, char *argv[]) 
{
    int opt;
    while ((opt = getopt(argc, argv, "a")) != -1) {
        switch(opt){
            case 'a':
               show_all = 1;
               break;
            case 'l':
               long_format = 1;
               break;
            default:
               fprintf(stderr, "usage: %s [-a] [path]\n", argv[0]);
               return 1;
        }
    }

    const char *path = (optind < argc)? argv[optind] :  ".";

    DIR *dir = opendir(path);
    if(!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){
        if(!show_all && entry ->d_name[0] == '.') continue; // to skip the dot files  
        if( long_format)
            
        printf("%s\n", entry->d_name);  // d_name comes from the dereferencing the struct: (entry*).d_name
    }   
    closedir(dir);
    return 0;
}
