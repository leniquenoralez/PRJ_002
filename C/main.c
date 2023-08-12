#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

int IGNORE_DOT_AND_DOTDOT = 1;
int IGNORE_DOT_DIR = 1;
int SORT_BY_STATUS_LAST_CHANGED = 1;
/*
    Display Flags
    −s Display the number of file system blocks actually used by each file, in units of 512 bytes or BLOCKSIZE (see ENVIRONMENT) where partial units are rounded up to the next integer value. If the output is to a terminal, a total sum for all the file sizes is output on a line before the listing.
    −F Display a slash ( ‘/’ ) immediately after each pathname that is a directory, an asterisk ( ‘∗’ ) after each that is executable, an at sign ( ‘@’ ) after each symbolic link, a percent sign ( ‘%’ ) after each whiteout, an equal sign ( ‘=’ ) after each socket, and a vertical bar ( ‘|’ ) after each that is a FIFO.
*/

/*
    Modifier Flags
    −h Modifies the −s and −l options, causing the sizes to be reported in bytes displayed in a human readable format. Overrides −k.
    −k Modifies the −s option, causing the sizes to be reported in kilobytes. The rightmost of the −k and

*/

/*
    Directory Flags
    −A List all entries except for ‘.’ and ‘..’. Always set for the super-user.
    −a Include directory entries whose names begin with a dot ( ‘.’ ) .
*/
/*
    Printing Flags
    −i For each file, print the file’s file serial number (inode number).
    −l (The lowercase letter “ell”). List in long format. (See below.)
    −n The same as −l, except that the owner and group IDs are displayed numerically rather than converting to a owner or group name.
    −R Recursively list subdirectories encountered.
    −d Directories are listed as plain files (not searched recursively) and symbolic links in the argumentlist are not indirected through.
*/
/*
    Override Flags
    −h flags overrides the previous flag. See also −h.
    −q Force printing of non-printable characters in file names as the character ‘?’; this is the default when output is to a terminal.

    −w Force raw printing of non-printable characters. This is the default when output is not to a terminal. The −w, and −q options override each other; the last one specified determines the format used for non-printable characters.
    The −l and −n options override each other, the last one specified determines the format used.
    The −c and −u options override each other; the last one specified determines the file time used.
    By default, ls lists one entry per line to standard output.  File information is displayed with one or more 〈blank〉 separating the information associated with the −i, −s, and −l options.
*/
/*
Sorting Flags
−f Output is not sorted.
−c Use time when file status was last changed, instead of time of last modification of the file for sorting ( −t ) or printing ( −l ) .
−r Reverse the order of the sort to get reverse lexicographical order or the smallest or oldest entries first.
−S Sort by size, largest file first.
−t Sort by time modified (most recently modified first) before sorting the operands by lexicographical
order.
−u Use time of last access, instead of last modification of the file for sorting ( −t ) or printing ( −l ) .
*/

int SHOW_DOT_DIRECTORY = 0;
int IGNORE_DOT_DOTDOT = 0;
int get_files_count(char *dirname)
{

    DIR *dir = opendir(dirname);
    if (dir == NULL)
    {
        perror("Error opening directory");
        return 1;
    }

    int num_files = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if ((!SHOW_DOT_DIRECTORY && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) || (!SHOW_DOT_DIRECTORY && entry->d_name[0] == '.'))
        {
            continue;
        }
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR)
        {
            num_files++;
        }
    }

    closedir(dir);
    return num_files;
}
int process_files(int num_files, char **files){
    for (size_t i = 0; i < num_files; i++)
    {
        printf("%s\t", files[i]);
    }
    return 0;
}


char ** allocate_memory(int num_files){
    char **files_memory = (char **)malloc(num_files * sizeof(char *));
    for (int i = 0; i < num_files; i++)
    {
        files_memory[i] = (char *)malloc(20 * sizeof(char));
        if (files_memory[i] == NULL)
        {
            perror("Memory allocation failed");
            return NULL;
        }
    }

    return files_memory;
}

int queue_dir(char *filename)
{

    struct stat file_stat;
    if (stat(filename, &file_stat) > 0)
    {
        perror("Error");
    }
    int num_files = get_files_count(filename);
    char **files = allocate_memory(num_files);

    DIR *dir = opendir(filename);
    if (dir == NULL)
    {
        printf("Unable to open %s\n.", filename);
        return -1;
    }
    struct dirent *entry;

    nlink_t num_links = file_stat.st_nlink;
    int files_in_dir = (unsigned long)num_links;
    int processed_file = 0;
    for (size_t i = 0; i < files_in_dir; i++)
    {
        entry = readdir(dir);
        if ((!SHOW_DOT_DIRECTORY && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) || (!SHOW_DOT_DIRECTORY && entry->d_name[0] == '.'))
        {
            continue;
        }
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR)
        {

            strcpy(files[processed_file], entry->d_name);
            processed_file++;
        }
    }
    
    (void)closedir(dir);

    process_files(num_files, files);
    free(files);

    return 0;
}

int lsFile(char *filename){
    printf("listing details for file: %s\n", filename);
    return 0;
}
void decodeFlags(int argc, char **argv)
{
    int c;

    while ((c = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1)
    {
        switch (c)
        {
        case 'A':
            IGNORE_DOT_DOTDOT = 1;
            break;
        case 'a':
            SHOW_DOT_DIRECTORY = 1;
            break;
        case 'c':
        case 'd':
        case 'F':
        case 'f':
        case 'h':
        case 'i':
        case 'k':
        case 'l':
        case 'n':
        case 'q':
        case 'R':
        case 'r':
        case 'S':
        case 's':
        case 't':
        case 'u':
        case 'w':
        default:
            printf("==> %c\n", c);
            break;
        }
    }
}
int main(int argc, char **argv){

    decodeFlags(argc, argv);

    // ls
    if (argc == 1)
    {
        queue_dir(".");
        return 1;
    }
    
    
    return -1;
}