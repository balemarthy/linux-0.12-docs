# System Wait Operations Header File (`sys/wait.h`)

## Overview

This header file defines the interface for process control and waiting operations in a UNIX-like operating system. It provides essential macros and function declarations that allow programs to monitor and respond to the state changes of child processes.

## Header Structure

The file follows standard C header file practices with include guards to prevent multiple inclusions:


#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>

// Content...

#endif


## Utility Macros

These macros extract specific bytes from a value:


#define _LOW(v)     ( (v) & 0377)      /* Extract low byte (8 bits) */
#define _HIGH(v)    ( ((v) >> 8) & 0377) /* Extract high byte */


The `0377` value is an octal representation of `255` (`0xFF` in hex), used for masking to get only the desired 8 bits.

## Wait Operation Options


/* options for waitpid, WUNTRACED not supported */
#define WNOHANG     1   /* Don't block if no child has exited */
#define WUNTRACED   2   /* Report status of stopped children */


- **`WNOHANG`**: When specified, `waitpid()` will not block if there are no children that have terminated.
- **`WUNTRACED`**: Would normally report the status of children that are stopped (but according to the comment, this is not supported in this implementation).

## Status Interpretation Macros

These macros help interpret the status values returned by `wait()` and `waitpid()` functions:


#define WIFEXITED(s)    (!((s)&0xFF))         /* True if child exited normally */
#define WIFSTOPPED(s)   (((s)&0xFF)==0x7F)    /* True if child is stopped */
#define WEXITSTATUS(s)  (((s)>>8)&0xFF)       /* Return exit status of child */
#define WTERMSIG(s)     ((s)&0x7F)            /* Return signal that terminated child */
#define WCOREDUMP(s)    ((s)&0x80)            /* True if child produced a core dump */
#define WSTOPSIG(s)     (((s)>>8)&0xFF)       /* Return stop signal */
#define WIFSIGNALED(s)  (((unsigned int)(s)-1 & 0xFFFF) < 0xFF) /* True if child exited due to signal */


### How Status Information is Encoded

The status value returned by wait functions packs multiple pieces of information into a single integer:

- **Exit Status**: Stored in the high byte (bits 8-15)
- **Signal Information**: Stored in the low byte (bits 0-7)
- **Core Dump Flag**: Indicated by bit 7 (0x80)

Each macro extracts specific information from this packed format:

- **`WIFEXITED`**: Returns true if the child process exited normally (by calling `exit()` or returning from `main()`).
- **`WIFSTOPPED`**: Returns true if the child process was stopped by a signal.
- **`WEXITSTATUS`**: Extracts the exit code when a process terminated normally.
- **`WTERMSIG`**: Returns the signal number that caused the process to terminate.
- **`WCOREDUMP`**: Checks if the terminating process produced a core dump.
- **`WSTOPSIG`**: Returns the signal that caused the process to stop.
- **`WIFSIGNALED`**: Returns true if the child process was terminated by a signal.

## Function Declarations


pid_t wait(int *stat_loc);
pid_t waitpid(pid_t pid, int *stat_loc, int options);


### `wait()` Function

**Purpose**: Suspends execution of the calling process until one of its child processes terminates.

**Parameters**:
- `stat_loc`: Pointer to an integer where the status information of the terminated child will be stored.

**Return Value**: Returns the process ID of the terminated child or -1 on error.

### `waitpid()` Function

**Purpose**: Provides more control over which child processes to wait for and whether to block.

**Parameters**:
- `pid`: Specifies which child processes to wait for:
  - `pid > 0`: Wait for the specific child with that PID
  - `pid = 0`: Wait for any child in the same process group
  - `pid < -1`: Wait for any child whose process group ID equals the absolute value of `pid`
  - `pid = -1`: Wait for any child process
- `stat_loc`: Pointer to store the status information
- `options`: Modifies the behavior (e.g., `WNOHANG` for non-blocking wait)

**Return Value**: 
- Returns the process ID of the child whose state has changed
- Returns 0 if `WNOHANG` was specified and no child was available
- Returns -1 on error

## Usage Examples

### Basic Child Process Monitoring


#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        printf("Child executing\n");
        return 42;  // Exit with status 42
    } else {
        // Parent process
        int status;
        pid_t child_pid = wait(&status);
        
        if (WIFEXITED(status)) {
            printf("Child %d exited with status %d\n", 
                    child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child terminated by signal %d\n", 
                    WTERMSIG(status));
        }
    }
    
    return 0;
}


### Non-blocking Wait with Multiple Children


#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid1, pid2;
    
    pid1 = fork();
    if (pid1 == 0) {
        sleep(2);
        return 1;
    }
    
    pid2 = fork();
    if (pid2 == 0) {
        sleep(1);
        return 2;
    }
    
    // Parent process monitors both children
    int remaining = 2;
    while (remaining > 0) {
        int status;
        pid_t finished = waitpid(-1, &status, WNOHANG);
        
        if (finished > 0) {
            printf("Child %d finished with status %d\n", 
                   finished, WEXITSTATUS(status));
            remaining--;
        } else {
            printf("No child exited, continuing work...\n");
            usleep(500000);  // Sleep for 0.5 seconds
        }
    }
    
    return 0;
}


## Implementation Notes

This header follows POSIX standards for process control. The macros are carefully designed to extract information from the status integer returned by wait functions, which compactly encodes multiple pieces of information about how a process terminated.

The bit manipulation in these macros reflects the internal representation of process states in the kernel, making this header a critical interface between user programs and the operating system's process management functionality.

