#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <dirent.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// mode_t is a typedef in sys/stat, it's basically an unsigned int 
// this part is to check the first bit which is the file type bit
void mode_string(mode_t mode, char *str){    //mode is just an Integer bitmask returned by kernel
    if (S_ISDIR(mode))         str[0] = 'd'; //if is it directory set it to d
    else if (S_ISLNK(mode))    str[0] = 'l'; //if is it simlink set it l
    else if (S_ISCHR(mode))    str[0] = 'c'; //if is it char device set it to c
    else if (S_ISBLK(mode))    str[0] = 'b'; //if is it block set it to b
    else if (S_ISFIFO(mode))   str[0] = 'p'; //if is it a FIFO set it to p
    else if (S_ISSOCK(mode))   str[0] = 's'; //if is it a socket set it to s 
    else                       str[0] = '-'; //if none then just dash

    // printf("%u\n", mode);

    // builidng the permisision string using bitwise & operator 
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    // printf("%u\n", mode & S_IRUSR);
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

void print_long(const char *dir, const char *name){
    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);

    struct stat st;
    if(lstat(fullpath, &st) < 0){
        perror(name);
        return;
    }

    char modes[11];
    mode_string(st.st_mode,modes);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char timebuf[64];

    struct tm *tm = localtime(&st.st_mtim.tv_sec);
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

    printf(
            "%s %lu %s %s %ld %s %s\n",
            modes,
            (unsigned long)st.st_nlink,
            user, 
            group, 
            (long)st.st_size,
            timebuf,
            name
          );
}

// global bool var for differnt option flags
int show_all = 0;       // show hidden files
int long_format = 0;    // show long format (sys stat info)

int main(int argc, char *argv[]) 
{
    int opt;
    while ((opt = getopt(argc, argv, "al")) != -1) {
        switch(opt){
            case 'a':
                show_all = 1;
                break;
            case 'l':
                long_format = 1;
                break;
            default:
                fprintf(stderr, "usage: %s [-al] [path]\n", argv[0]);
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
        if(long_format){
            print_long(path, entry->d_name);
        }
        else {
            printf("%s\n", entry->d_name);  // d_name comes from the dereferencing the struct: (entry*).d_name
        }

    }   
    closedir(dir);
    return 0;
}
