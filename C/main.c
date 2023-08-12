#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

int IGNORE_DOT_AND_DOTDOT = 1;
int IGNORE_DOT_DIR = 1;
int SORT_BY_STATUS_LAST_CHANGED = 1;

static enum {
    DEFAULT = 1,
    TIME_STATUS_LAST_CHANGED,
    TIME_LAST_MODIFIED,
    TIME_LAST_ACCESS,
    FILE_SIZE
} SORT_MODE;

int lsDir(char *filename)
{

    DIR *currDir = opendir(filename);
    struct dirent *dirp;
    if (currDir == NULL)
    {
        printf("Unable to open %s\n.", filename);
        return -1;
    }
    int showDotFiles = hasFlag('a');
    while ((dirp = readdir(currDir)) != NULL)
    {
        struct stat currentFileStat;
        if (lstat(filename, &currentFileStat))
        {
            fprintf(stderr, "failed getting file info");
            return -1;
        }
        if (showDotFiles == -1 && dirp->d_name[0] == '.')
        {
            continue;
        } else {
            printf("%s\t", dirp->d_name);
        }
        
        
    }
    (void)closedir(currDir);

    return 0;
}

int lsFile(char *filename){
    printf("listing details for file: %s\n", filename);
    return 0;
}

int main(int argc, char **argv){
    int c;

    while ((c = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1)
    {
        printf("==> %c\n", c);
    }
    return -1;
}