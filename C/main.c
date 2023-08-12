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
    −R Recursively list subdirectories encountered.
    −d Directories are listed as plain files (not searched recursively) and symbolic links in the argumentlist are not indirected through.
*/
/*
    Printing Flags
    −i For each file, print the file’s file serial number (inode number).
    −l (The lowercase letter “ell”). List in long format. (See below.)
    −n The same as −l, except that the owner and group IDs are displayed numerically rather than converting to a owner or group name.
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