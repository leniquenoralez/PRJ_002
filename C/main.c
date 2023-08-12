#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
char *CURRENT_DIRECTORY = NULL;
int IGNORE_DOT_AND_DOTDOT = 1;
int IGNORE_DOT_DIR = 1;
static enum SORT_MODES {
    LAST_MODIFIED = 0,
    STATUS_CHANGED,
    LAST_ACCESSED,
    FILE_SIZE,
    REVERSE_SORT
} SORT_MODE;
static enum FORMAT_MODES {
    DEFAULT = 0,
    LONG,
    NUMERIC
} FORMAT_MODE;
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
    Printing Flags
    −i For each file, print the file’s file serial number (inode number).
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
char can_read(mode_t file_mode, char mode_type) {
    mode_t mode;
    if (mode_type == 'U')
    {
        mode = S_IRUSR;
    }
    else if (mode_type == 'G')
    {
        mode = S_IRGRP;
    }
    else if (mode_type == 'O')
    {
        mode = S_IROTH;
    }
    else
    {
        perror("Unknown mode type!");
        return 1;
    }

    return (file_mode & mode) ? 'r' : '-';
}
char can_write(mode_t file_mode, char mode_type)
{
    mode_t mode;
    if (mode_type == 'U')
    {
        mode = S_IWUSR;
    }
    else if (mode_type == 'G')
    {
        mode = S_IWGRP;
    }
    else if (mode_type == 'O')
    {
        mode = S_IWOTH;
    }
    else
    {
        perror("Unknown mode type!");
        return 1;
    }

    return (file_mode & mode) ? 'w' : '-';
}
char can_execute(mode_t file_mode, char mode_type)
{
    mode_t mode;

    if (mode_type == 'U')
    {
        mode = S_IXUSR;
    }
    else if (mode_type == 'G')
    {
        mode = S_IXGRP;
    }
    else if (mode_type == 'O')
    {
        mode = S_IXOTH;
    }
    else
    {
        perror("Unknown mode type!");
        return 1;
    }
    return (file_mode & mode) ? 'x' : '-';
}

char get_file_type(mode_t file_mode)
{
    switch (file_mode & S_IFMT)
    {
    case S_IFREG:
        return '-';
    case S_IFDIR:
        return 'd';
    case S_IFLNK:
        return 'l';
    case S_IFCHR:
        return 'c';
    case S_IFBLK:
        return 'b';
    case S_IFIFO:
        return 'p';
    case S_IFSOCK:
        return 's';
    case S_IFWHT:
        return 'w'; 
    default:
        return 'u';
    }
}

char *get_file_modes(mode_t file_mode)
{
    char *file_modes = (char *)malloc(11 * sizeof(char));

    char user_mode = 'U';
    char group_mode = 'G';
    char other_mode = 'O';

    file_modes[0] = get_file_type(file_mode);

    file_modes[1] = can_read(file_mode, user_mode);
    file_modes[2] = can_write(file_mode, user_mode);
    file_modes[3] = can_execute(file_mode, user_mode);

    file_modes[4] = can_read(file_mode, group_mode);
    file_modes[5] = can_write(file_mode, group_mode);
    file_modes[6] = can_execute(file_mode, group_mode);

    file_modes[7] = can_read(file_mode, other_mode);
    file_modes[8] = can_write(file_mode, other_mode);
    file_modes[9] = can_execute(file_mode, other_mode);
    file_modes[10] = '\0';
    return file_modes;
}
char *get_owner(struct stat file_stat)
{
    uid_t user_id = file_stat.st_uid;
    gid_t group_id = file_stat.st_gid;

    struct passwd *user_info = getpwuid(user_id);
    struct group *group_info = getgrgid(group_id);
    char *user = (char *)malloc(1024 * sizeof(char));
    sprintf(user, "%s %s", user_info->pw_name, group_info->gr_name);
    return user;
}
char *get_full_file_path(char *filename){
    char *filePath = (char *)malloc(1024 * sizeof(char));
    if (CURRENT_DIRECTORY == NULL)
    {
        sprintf(filePath, "%s", filename);
    }
    else
    {
        sprintf(filePath, "%s/%s", CURRENT_DIRECTORY, filename);
    }
    return filePath;
}
char *get_modified_date_time(struct tm *time_info)
{
    int day = time_info->tm_mday;
    int hour = time_info->tm_hour;
    int minute = time_info->tm_min;
    char *month_abbrev = (char *)malloc(4 * sizeof(char));
    char *formatted_time = (char *)malloc(13 * sizeof(char));

    strftime(month_abbrev, sizeof(month_abbrev), "%b", time_info);

    sprintf(formatted_time, "%s %2d %02d:%02d", month_abbrev, day,hour, minute);
    return formatted_time;
}
int print_long_format(char *filename){
    struct stat file_stat;

    char *filePath = get_full_file_path(filename);

    if (stat(filePath, &file_stat) < 0)
    {
        char *error_message = (char *)malloc(1024 * sizeof(char));
        sprintf(error_message, "Error %s:", filePath);
        perror(error_message);
        free(error_message);
    }

    char *file_mode = get_file_modes(file_stat.st_mode);
    nlink_t num_links = file_stat.st_nlink;
    char *owner = get_owner(file_stat);
    off_t file_size = file_stat.st_size;
    time_t mod_time = file_stat.st_mtime;
    struct tm *time_info = localtime(&mod_time);
    char *last_modified_date_time = get_modified_date_time(time_info);

    printf("%s %2d %s %6lld %s %s\n", file_mode, num_links, owner, (long long)file_size, last_modified_date_time, filename);
    free(owner);
    free(file_mode);
    free(last_modified_date_time);
    free(filePath);
    return 1;
}
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
        if (FORMAT_MODE == LONG)
        {
            print_long_format(files[i]);
        }
        else
        {
            printf("%s\t", files[i]);
        }
    }
    return 0;
}

int last_modified_compare(const void *a, const void *b)
{
    char *file_a = *(char **)a;
    char *file_b = *(char **)b;

    struct stat file_a_stat;
    struct stat file_b_stat;

    char *file_a_Path = get_full_file_path(file_a);
    char *file_b_Path = get_full_file_path(file_b);

    if (stat(file_a_Path, &file_a_stat) > 0)
    {
        perror("Error");
    }
    if (stat(file_b_Path, &file_b_stat) > 0)
    {
        perror("Error");
    }
    time_t mod_time1 = file_a_stat.st_mtime;
    time_t mod_time2 = file_b_stat.st_mtime;
    free(file_a_Path);
    free(file_b_Path);

    if (mod_time1 > mod_time2)
    {
        return 1;
    }
    else if (mod_time2 > mod_time1)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
char ** allocate_memory(int count){
    char **allocated_memory = (char **)malloc(count * sizeof(char *));
    for (int i = 0; i < count; i++)
    {
        allocated_memory[i] = (char *)malloc(1024 * sizeof(char));
        if (allocated_memory[i] == NULL)
        {
            perror("Memory allocation failed");
            return NULL;
        }
    }

    return allocated_memory;
}
void sort_files(char **files, int num_files)
{
    if (SORT_MODE == LAST_MODIFIED)
    {
        qsort(files, num_files, sizeof(files[0]), last_modified_compare);
    }
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
    sort_files(files, num_files);
    process_files(num_files, files);
    free(files);

    return 0;
}

int lsFile(char *filename){
    printf("listing details for file: %s\n", filename);
    return 0;
}
int decodeFlags(int argc, char **argv)
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
        case 'l':
            FORMAT_MODE = LONG;
            break;
        case 'n':
            FORMAT_MODE = NUMERIC;
            break;
        case 'c':
        case 'd':
        case 'F':
        case 'f':
        case 'h':
        case 'i':
        case 'k':
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
    return c;
}

int main(int argc, char **argv){

    SORT_MODE = LAST_MODIFIED;
    decodeFlags(argc, argv);
    int mode = argc - optind;

    if (mode == 0)
    {
        queue_dir(".");
        return 1;
    } 
    if (mode == 1)
    {
        char *filename = argv[optind];
        CURRENT_DIRECTORY = (char *)malloc(1024 * sizeof(char));
        sprintf(CURRENT_DIRECTORY, "%s", filename);
        queue_dir(filename);
        free(CURRENT_DIRECTORY);
        return 1;
    }
    if (mode >= 2)
    {
        for (size_t i = optind; i < argc; i++)
        {
            char *filename = argv[i];
            CURRENT_DIRECTORY = (char *)malloc(1024 * sizeof(char));
            sprintf(CURRENT_DIRECTORY, "%s", filename);
            printf("%s: \n", filename);
            queue_dir(filename);
            if (i != argc - 1)
            {
                printf("\n\n");
            }
            free(CURRENT_DIRECTORY);
        }

        return 1;
    }
    return -1;
}