# Linux Error Number Definitions (`errno.h`)

## Overview

This header file defines standard error codes used in Linux systems. Error codes are integral to system programming as they provide a standardized way for functions to communicate failure reasons to calling code. When a system call or library function encounters an error, it typically sets the global variable `errno` to indicate what went wrong.

## Error Number Implementation

The file establishes a collection of named constants that represent different error conditions that can occur during program execution. These error codes follow POSIX standards and are used consistently across Unix-like operating systems.


#ifndef _ERRNO_H
#define _ERRNO_H

extern int errno;

/* General error codes */
#define ERROR           99
#define EPERM           1
#define ENOENT          2
#define ESRCH           3
/* ... more error codes ... */


### Key Components

#### Header Guard


#ifndef _ERRNO_H
#define _ERRNO_H
// ... code ...
#endif


This standard include guard prevents multiple inclusions of the header file, which could cause redefinition errors during compilation. If `_ERRNO_H` is already defined (meaning the file has been included before), the preprocessor skips everything between the `#ifndef` and `#endif`.

#### External Variable Declaration


extern int errno;


This line declares the global `errno` variable as external, meaning it's defined elsewhere in the system libraries. Programs can access this variable to determine what error occurred after a function call fails.

#### Error Code Definitions

The bulk of the file consists of `#define` directives that associate meaningful names with numeric error codes. For example:


#define EPERM           1  /* Operation not permitted */
#define ENOENT          2  /* No such file or directory */
#define ESRCH           3  /* No such process */


## Common Error Codes and Their Meanings

| Error Code | Value | Description |
|------------|-------|-------------|
| `EPERM` | 1 | Operation not permitted |
| `ENOENT` | 2 | No such file or directory |
| `ESRCH` | 3 | No such process |
| `EINTR` | 4 | Interrupted system call |
| `EIO` | 5 | Input/output error |
| `ENXIO` | 6 | No such device or address |
| `E2BIG` | 7 | Argument list too long |
| `ENOEXEC` | 8 | Exec format error |
| `EBADF` | 9 | Bad file descriptor |
| `ECHILD` | 10 | No child processes |
| `EAGAIN` | 11 | Resource temporarily unavailable |
| `ENOMEM` | 12 | Cannot allocate memory |
| `EACCES` | 13 | Permission denied |
| `EFAULT` | 14 | Bad address |

## Special Categories of Error Codes

### File and Directory Operations
Errors like `ENOENT` (file not found), `ENOTDIR` (not a directory), and `EISDIR` (is a directory) help diagnose file operation issues.

### Resource Limitations
Errors such as `ENOMEM` (out of memory), `ENOSPC` (no space left on device), and `EMFILE` (too many open files) indicate resource exhaustion.

### Permission and Access Control
Errors including `EACCES` (permission denied) and `EPERM` (operation not permitted) relate to security restrictions.

### Kernel-Only Error Codes


/* Should never be seen by user programs */
#define ERESTARTSYS     512
#define ERESTARTNOINTR  513


These special error codes are used internally by the Linux kernel to manage system call restarts. They're not meant to be exposed to user programs and typically indicate that a system call should be transparently restarted after being interrupted.

## Using Error Codes in Programs

When a system call fails, it typically returns -1 and sets `errno`. Programmers check for this condition and then examine `errno` to determine the cause:


#include <stdio.h>
#include <errno.h>
#include <string.h>

int main() {
    FILE *file = fopen("nonexistent_file.txt", "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", strerror(errno));
        // errno will likely be ENOENT (2) - No such file or directory
    }
    return 0;
}


The `strerror()` function, mentioned in the original header comment, converts error numbers into human-readable messages.

## Historical Context

The error numbers in Linux were originally borrowed from Minix (an educational operating system) and have since evolved to conform with POSIX standards. This standardization ensures compatibility across different Unix-like systems, making code more portable.

These error codes represent decades of operating system design wisdom, capturing the common failure modes that occur in system programming.

