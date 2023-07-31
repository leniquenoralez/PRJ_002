#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
int isFlagValid(char *flag)
{
    char flags[19] = "AacdFfhiklnqRrSstuw";

    for (__SIZE_TYPE__ i = 0; i < 19; i++)
    {
        if (*flag == flags[i])
        {
            return 0;
        }
    }

    return -1;
}
int allFlagsValid(char *flags)
{
    int flagsCount = strlen(flags);
    for (size_t i = 1; i < flagsCount; i++)
    {
        if (isFlagValid(&flags[i]) < 0)
        {
            printf("%c is not a valid flag!!\n", flags[i]);
            return -1;
        }
    }
    return 0;
}


int lsDir(char *filename){

    DIR *currDir = opendir(filename);
    struct dirent *dirp;
    if (currDir == NULL)
    {
        printf("Unable to open %s\n.", filename);
        return -1;
    }
    while ((dirp = readdir(currDir)) != NULL)
    {
        printf("%s\n", dirp->d_name);
    }
    (void)closedir(currDir);

    return 0;
}

int lsFile(char *filename){
    printf("listing details for file: %s\n", filename);
    return 0;
}

int main(int argc, char **argv){
    struct stat fileStat;


    if (argc == 1)
    {
        printf("no args or filename provided. listing current directory.\n");
        return lsDir(".");
    }
    char *firstArg = argv[1];

    if (argc == 2 && firstArg[0] == '-' && allFlagsValid(firstArg) == 0)
    {
        char *flags = firstArg;
        printf("Flags provided: %s\n", flags);
        return lsDir(".");
 
    }

    if (lstat(firstArg, &fileStat))
    {
        fprintf(stderr, "failed getting file info");
        return -1;
    }
    if (argc == 2 && S_ISREG(fileStat.st_mode))
    {
        return lsFile(firstArg);

    }
    
    if (argc == 2 && S_ISDIR(fileStat.st_mode))
    {
        return lsDir(firstArg);
    }

    char *flags = argv[1];
    char *filename = argv[2];
    if (allFlagsValid(flags) < 0)
    {
        return -1;
    }
    if (lstat(filename, &fileStat))
    {
        fprintf(stderr, "failed getting file info");
        return -1;
    }
    if (S_ISREG(fileStat.st_mode))
    {
        return lsFile(filename);
    }

    if (S_ISDIR(fileStat.st_mode)){
        return lsDir(filename);
    }
    return -1;
}