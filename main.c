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

char **REG_FILES = NULL;
char **DIRECTORIES = NULL;
char **SYMBOLIC_LINKS = NULL;
char **CHARACTER_SPECIAL_FILES = NULL;
char **BLOCK_SPECIAL_FILES = NULL;
char **FIFO_FILES = NULL;
char **SOCKETS = NULL;
char **WHITEOUT_FILES = NULL;
char **UNKNOWN_FILES = NULL;

int IGNORE_DOT_AND_DOTDOT = 1;
int IGNORE_DOT_DIR = 1;
int REG_FILES_COUNT = 0;
int DIRS_COUNT = 0;
int FILES_COUNT[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

int REG_FILE_TYPE_COUNT = 0;
int DIR_FILE_TYPE_COUNT = 1;
int SYMBOLIC_LINK_FILE_TYPE_COUNT = 2;
int CHARACTER_FILE_TYPE_COUNT = 3;
int BLOCK_FILE_TYPE_COUNT = 4;
int FIFO_FILE_TYPE_COUNT = 5;
int SOCKET_FILE_TYPE_COUNT = 6;
int WHITEOUT_FILE_TYPE_COUNT = 7;
int UNKNOWN_FILES_TYPE_COUNT = 8;

static enum SORT_MODES {
    ALPHABETICALLY = 0,
    LAST_MODIFIED,
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

enum FILE_TYPES
{
    REGULAR_FILE = '-',
    DIRECTORY = 'd',
    SYMBOLIC_LINK = 'l',
    CHARACTER_SPECIAL = 'c',
    BLOCK_SPECIAL = 'b',
    FIFO = 'p',
    SOCKET = 's',
    WHITEOUT = 'w',
    UNKNOWN = 'u',
};

int SHOW_DOT_DIRECTORY = 0;
int IGNORE_DOT_DOTDOT = 0;
char can_read(mode_t file_mode, char mode_type)
{
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
char *get_full_file_path(char *filename)
{
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
char get_file_type(char *filename)
{
    struct stat file_stat;

    char *filePath = get_full_file_path(filename);

    if (stat(filePath, &file_stat) < 0)
    {
        char *error_message = (char *)malloc(1024 * sizeof(char));
        sprintf(error_message, "Error %s:", filePath);
        perror(error_message);
        free(error_message);
    }
    mode_t file_mode = file_stat.st_mode;
    free(filePath);

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

char *get_file_modes(char *filename)
{
    struct stat file_stat;

    char *filePath = get_full_file_path(filename);

    if (stat(filePath, &file_stat) < 0)
    {
        char *error_message = (char *)malloc(1024 * sizeof(char));
        sprintf(error_message, "Error %s:", filePath);
        perror(error_message);
        free(error_message);
    }
    char *file_modes = (char *)malloc(11 * sizeof(char));

    char user_mode = 'U';
    char group_mode = 'G';
    char other_mode = 'O';
    mode_t file_mode = file_stat.st_mode;
    free(filePath);

    file_modes[0] = get_file_type(filename);

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

char *get_modified_date_time(struct tm *time_info)
{
    int day = time_info->tm_mday;
    int hour = time_info->tm_hour;
    int minute = time_info->tm_min;
    char *month_abbrev = (char *)malloc(4 * sizeof(char));
    char *formatted_time = (char *)malloc(13 * sizeof(char));

    strftime(month_abbrev, sizeof(month_abbrev), "%b", time_info);

    sprintf(formatted_time, "%s %2d %02d:%02d", month_abbrev, day, hour, minute);
    return formatted_time;
}
int print_long_format(char *filename)
{
    struct stat file_stat;

    char *filePath = get_full_file_path(filename);

    if (stat(filePath, &file_stat) < 0)
    {
        char *error_message = (char *)malloc(1024 * sizeof(char));
        sprintf(error_message, "Error %s:", filePath);
        perror(error_message);
        free(error_message);
    }

    char *file_mode = get_file_modes(filename);
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
        char *error_message = (char *)malloc(1024 * sizeof(char));
        sprintf(error_message, "Error opening directory: %s", dirname);
        perror(error_message);
        free(error_message);
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
int process_files(int num_files, char **files)
{
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
int strings_compare(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}
char **allocate_memory(int count)
{
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

void free_memory(char **array, int size)
{
    for (int i = 0; i < size; i++)
    {
        free(array[i]);
    }

    free(array);
}
void sort_files(char **files, int num_files)
{
    if (SORT_MODE == LAST_MODIFIED)
    {
        qsort(files, num_files, sizeof(files[0]), last_modified_compare);
    }
    if (SORT_MODE == ALPHABETICALLY)
    {
        qsort(files, num_files, sizeof(files[0]), strings_compare);
    }
}
void sort_dirs(char **files, int num_files)
{
    qsort(files, num_files, sizeof(files[0]), strings_compare);
}
int ls_dir(char *filename)
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
    free_memory(files, num_files);

    return 0;
}
void ls_reg_file(char *filename)
{
    switch (FORMAT_MODE)
    {
    case LONG:
        print_long_format(filename);
        break;
    default:
        printf("%s\t", filename);
        break;
    }
}
void ls_reg_files(char **reg_files)
{
    for (size_t i = 0; i < FILES_COUNT[REG_FILE_TYPE_COUNT]; i++)
    {
        ls_reg_file(reg_files[i]);
        
    }
    if (FILES_COUNT[REG_FILE_TYPE_COUNT] != 0)
    {
        printf("\n\n");
    }
}
int lsFile(char *filename)
{
    printf("listing details for file: %s\n", filename);
    return 0;
}
void set_file_types_count(char **files, int files_count, char start_index)
{
    for (size_t i = start_index; i <= files_count; i++)
    {
        enum FILE_TYPES file_type = get_file_type(files[i]);
        switch (file_type)
        {
        case REGULAR_FILE:
            FILES_COUNT[REG_FILE_TYPE_COUNT]++;
            break;
        case DIRECTORY:
            FILES_COUNT[DIR_FILE_TYPE_COUNT]++;
            break;
        case SYMBOLIC_LINK:
            FILES_COUNT[SYMBOLIC_LINK_FILE_TYPE_COUNT]++;
            break;
        case CHARACTER_SPECIAL:
            FILES_COUNT[CHARACTER_FILE_TYPE_COUNT]++;
            break;
        case BLOCK_SPECIAL:
            FILES_COUNT[BLOCK_FILE_TYPE_COUNT]++;
            break;
        case FIFO:
            FILES_COUNT[FIFO_FILE_TYPE_COUNT]++;
            break;
        case SOCKET:
            FILES_COUNT[SOCKET_FILE_TYPE_COUNT]++;
            break;
        case WHITEOUT:
            FILES_COUNT[WHITEOUT_FILE_TYPE_COUNT]++;
            break;
        default:
            FILES_COUNT[UNKNOWN_FILES_TYPE_COUNT]++;
            break;
        }
    }
}
void allocate_memory_for_file_types()
{
    if (FILES_COUNT[REG_FILE_TYPE_COUNT] > 0)
    {
        REG_FILES = allocate_memory(FILES_COUNT[REG_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[DIR_FILE_TYPE_COUNT] > 0)
    {
        DIRECTORIES = allocate_memory(FILES_COUNT[DIR_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[SYMBOLIC_LINK_FILE_TYPE_COUNT] > 0)
    {
        SYMBOLIC_LINKS = allocate_memory(FILES_COUNT[SYMBOLIC_LINK_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[CHARACTER_FILE_TYPE_COUNT] > 0)
    {
        CHARACTER_SPECIAL_FILES = allocate_memory(FILES_COUNT[CHARACTER_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[BLOCK_FILE_TYPE_COUNT] > 0)
    {
        BLOCK_SPECIAL_FILES = allocate_memory(FILES_COUNT[BLOCK_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[FIFO_FILE_TYPE_COUNT] > 0)
    {
        FIFO_FILES = allocate_memory(FILES_COUNT[FIFO_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[SOCKET_FILE_TYPE_COUNT] > 0)
    {
        SOCKETS = allocate_memory(FILES_COUNT[SOCKET_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[WHITEOUT_FILE_TYPE_COUNT] > 0)
    {
        WHITEOUT_FILES = allocate_memory(FILES_COUNT[WHITEOUT_FILE_TYPE_COUNT]);
    }
    if (FILES_COUNT[UNKNOWN_FILES_TYPE_COUNT] > 0)
    {
        UNKNOWN_FILES = allocate_memory(FILES_COUNT[UNKNOWN_FILES_TYPE_COUNT]);
    }
}
void set_files_by_type(char **files, int files_count, char start_index)
{

    int REG_FILE_COUNT = 0;
    int DIRECTORIES_COUNT = 0;
    for (size_t i = start_index; i <= files_count; i++)
    {
        char file_type = get_file_type(files[i]);
        if (file_type == REGULAR_FILE)
        {
            REG_FILES[REG_FILE_COUNT++] = strdup(files[i]);
        }
        if (file_type == DIRECTORY)
        {
            DIRECTORIES[DIRECTORIES_COUNT++] = strdup(files[i]);
        }
    }
};

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

int main(int argc, char **argv)
{

    SORT_MODE = ALPHABETICALLY;
    decodeFlags(argc, argv);
    int mode = argc - optind;

    if (argc == optind)
    {
        ls_dir(".");
        return 1;
    }
    set_file_types_count(argv, mode, optind);
    allocate_memory_for_file_types();
    set_files_by_type(argv, mode, optind);

    sort_files(REG_FILES, FILES_COUNT[REG_FILE_TYPE_COUNT]);
    sort_dirs(DIRECTORIES, FILES_COUNT[DIR_FILE_TYPE_COUNT]);

    ls_reg_files(REG_FILES);

    for (size_t i = 0; i < FILES_COUNT[DIR_FILE_TYPE_COUNT]; i++)
    {
        char *filename = DIRECTORIES[i];
        CURRENT_DIRECTORY = (char *)malloc(1024 * sizeof(char));
        sprintf(CURRENT_DIRECTORY, "%s", filename);
        if (FILES_COUNT[DIR_FILE_TYPE_COUNT] > 1 || FILES_COUNT[REG_FILE_TYPE_COUNT] > 0)
        {
            printf("%s: \n", filename);
        }
        ls_dir(filename);
        if (i != FILES_COUNT[DIR_FILE_TYPE_COUNT] - 1)
        {
            printf("\n\n");
        }
        free(CURRENT_DIRECTORY);
        CURRENT_DIRECTORY = NULL;
    }
    free_memory(REG_FILES, FILES_COUNT[REG_FILE_TYPE_COUNT]);
    free_memory(DIRECTORIES, FILES_COUNT[DIR_FILE_TYPE_COUNT]);

    return 1;
}