# Linux `sys/stat.h` Header File Guide

## Overview

The `sys/stat.h` header file defines the structure of the data returned by the `stat` family of functions, which are used to get information about files in a Linux/Unix system. This header also defines file types, permission bit masks, and several system calls for working with files and directories.

## The `stat` Structure

The core of this header is the `stat` structure, which stores detailed information about a file:


struct stat {
    dev_t   st_dev;     /* ID of device containing file */
    ino_t   st_ino;     /* Inode number */
    umode_t st_mode;    /* File type and permissions */
    nlink_t st_nlink;   /* Number of hard links */
    uid_t   st_uid;     /* User ID of owner */
    gid_t   st_gid;     /* Group ID of owner */
    dev_t   st_rdev;    /* Device ID (if special file) */
    off_t   st_size;    /* Total size in bytes */
    time_t  st_atime;   /* Time of last access */
    time_t  st_mtime;   /* Time of last modification */
    time_t  st_ctime;   /* Time of last status change */
};


When you call functions like `stat()`, `fstat()`, or similar system calls, they populate this structure with information about the specified file.

## File Type Macros

The header defines constants for identifying file types. These are used with the `st_mode` field in the `stat` structure.


#define S_IFMT  00170000   /* Bit mask for file type */
#define S_IFLNK  0120000   /* Symbolic link */
#define S_IFREG  0100000   /* Regular file */
#define S_IFBLK  0060000   /* Block device */
#define S_IFDIR  0040000   /* Directory */
#define S_IFCHR  0020000   /* Character device */
#define S_IFIFO  0010000   /* FIFO/pipe */


### Special Permission Bits

Beyond the file type, `st_mode` can also contain these special permission bits:


#define S_ISUID  0004000   /* Set-user-ID bit */
#define S_ISGID  0002000   /* Set-group-ID bit */
#define S_ISVTX  0001000   /* Sticky bit */


- **Set-user-ID (SUID)**: When applied to an executable file, it allows the file to be executed with the privileges of the file's owner
- **Set-group-ID (SGID)**: Similar to SUID, but uses the file's group permissions instead
- **Sticky bit**: Primarily used on directories to modify the deletion behavior of files within the directory

## File Type Testing Macros

The header provides convenient macros to test what type of file you're dealing with:


#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)  /* Is a symbolic link? */
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)  /* Is a regular file? */
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)  /* Is a directory? */
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)  /* Is a character device? */
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)  /* Is a block device? */
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)  /* Is a FIFO/pipe? */


These macros extract the file type from `st_mode` using a bitwise AND with `S_IFMT` (the file type mask), then compare the result with the respective file type constant.

## Permission Bit Macros

File permissions in Unix/Linux are defined using these bit masks:

### User Permissions (Owner)

#define S_IRWXU 00700   /* Read, write, and execute by owner */
#define S_IRUSR 00400   /* Read permission, owner */
#define S_IWUSR 00200   /* Write permission, owner */
#define S_IXUSR 00100   /* Execute/search permission, owner */


### Group Permissions

#define S_IRWXG 00070   /* Read, write, and execute by group */
#define S_IRGRP 00040   /* Read permission, group */
#define S_IWGRP 00020   /* Write permission, group */
#define S_IXGRP 00010   /* Execute/search permission, group */


### Other Permissions (World)

#define S_IRWXO 00007   /* Read, write, and execute by others */
#define S_IROTH 00004   /* Read permission, others */
#define S_IWOTH 00002   /* Write permission, others */
#define S_IXOTH 00001   /* Execute/search permission, others */


These permission bits are combined to create the familiar Unix permission modes (like 755, 644, etc.) that you see when using `ls -l`.

## System Calls and Functions

The header declares several important system calls for working with files:


extern int chmod(const char *_path, mode_t mode);
extern int fstat(int fildes, struct stat *stat_buf);
extern int mkdir(const char *_path, mode_t mode);
extern int mkfifo(const char *_path, mode_t mode);
extern int stat(const char *filename, struct stat *stat_buf);
extern mode_t umask(mode_t mask);


### Function Descriptions

- **chmod**: Changes the permissions of a file specified by path
- **fstat**: Gets file status information using a file descriptor
- **mkdir**: Creates a new directory with specified permissions
- **mkfifo**: Creates a named pipe (FIFO) with specified permissions
- **stat**: Gets file status information using a file path
- **umask**: Sets the file mode creation mask

## Using stat() in Practice

Here's an example of how you might use the `stat()` function:


#include <sys/stat.h>
#include <stdio.h>
#include <time.h>

int main() {
    struct stat file_info;
    
    if (stat("example.txt", &file_info) != 0) {
        perror("stat");
        return 1;
    }
    
    printf("File size: %ld bytes\n", file_info.st_size);
    
    // Check file type
    if (S_ISREG(file_info.st_mode)) {
        printf("Regular file\n");
    } else if (S_ISDIR(file_info.st_mode)) {
        printf("Directory\n");
    }
    
    // Check permissions
    printf("Permissions: ");
    printf((file_info.st_mode & S_IRUSR) ? "r" : "-");
    printf((file_info.st_mode & S_IWUSR) ? "w" : "-");
    printf((file_info.st_mode & S_IXUSR) ? "x" : "-");
    printf((file_info.st_mode & S_IRGRP) ? "r" : "-");
    printf((file_info.st_mode & S_IWGRP) ? "w" : "-");
    printf((file_info.st_mode & S_IXGRP) ? "x" : "-");
    printf((file_info.st_mode & S_IROTH) ? "r" : "-");
    printf((file_info.st_mode & S_IWOTH) ? "w" : "-");
    printf((file_info.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");
    
    // Display times
    printf("Last modified: %s", ctime(&file_info.st_mtime));
    
    return 0;
}


## Why This Approach?

The design of the `stat` structure and associated macros follows Unix's philosophy of simple, composable tools:

1. **Separation of metadata from content**: File information is kept separate from the actual file data
2. **Bitwise operations for efficiency**: Using bit masks and bitwise operations is efficient both in terms of storage and computation
3. **Consistent permission model**: The read/write/execute permission model is applied consistently across user/group/others

This approach has stood the test of time and remains fundamental to how modern Unix/Linux systems operate, despite being designed decades ago.

