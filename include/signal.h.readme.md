# Linux Signal Handling Interface

## Overview

This header file defines the signal handling interface for a Linux system. It specifies the standard POSIX signal constants, data structures, and function prototypes essential for implementing signal-based process communication and exception handling in Linux programs.

## Signal Basics

Signals are software interrupts that provide a mechanism for handling asynchronous events. This header establishes the foundation for signal handling in a Linux environment by defining:

- Signal types and data structures
- Standard signal constants
- Signal manipulation functions
- Signal action configuration

## Core Data Types


typedef int sig_atomic_t;

**Purpose**: Represents an atomic entity that can be accessed as an atomic operation, even in the presence of asynchronous interrupts caused by signals.


typedef unsigned int sigset_t;    /* 32 bits */

**Purpose**: Defines a signal set data type as a 32-bit unsigned integer where each bit represents a distinct signal.

## Signal Constants

The header defines the standard POSIX signals with their conventional numeric values:

| Signal | Value | Description |
|--------|-------|-------------|
| SIGHUP | 1 | Hangup detected on controlling terminal or death of controlling process |
| SIGINT | 2 | Interrupt from keyboard (Ctrl+C) |
| SIGQUIT | 3 | Quit from keyboard (Ctrl+\\) |
| SIGILL | 4 | Illegal instruction |
| SIGTRAP | 5 | Trace/breakpoint trap |
| SIGABRT/SIGIOT | 6 | Abort signal |
| SIGUNUSED | 7 | Unused signal |
| SIGFPE | 8 | Floating-point exception |
| SIGKILL | 9 | Kill signal (cannot be caught or ignored) |
| SIGUSR1 | 10 | User-defined signal 1 |
| SIGSEGV | 11 | Invalid memory reference |
| SIGUSR2 | 12 | User-defined signal 2 |
| SIGPIPE | 13 | Broken pipe: write to pipe with no readers |
| SIGALRM | 14 | Timer signal from alarm() |
| SIGTERM | 15 | Termination signal |
| SIGSTKFLT | 16 | Stack fault |
| SIGCHLD | 17 | Child stopped or terminated |
| SIGCONT | 18 | Continue if stopped |
| SIGSTOP | 19 | Stop process (cannot be caught or ignored) |
| SIGTSTP | 20 | Stop typed at terminal (Ctrl+Z) |
| SIGTTIN | 21 | Terminal input for background process |
| SIGTTOU | 22 | Terminal output for background process |

## Signal Action Flags


#define SA_NOCLDSTOP    1
#define SA_INTERRUPT    0x20000000
#define SA_NOMASK       0x40000000
#define SA_ONESHOT      0x80000000


These flags modify the behavior of signal handlers:

- **SA_NOCLDSTOP**: Do not generate SIGCHLD when child processes stop
- **SA_INTERRUPT**: Interrupt system calls when the signal handler returns
- **SA_NOMASK**: Don't block the current signal while its handler is executing
- **SA_ONESHOT**: Restore the signal action to the default after the handler executes

## Signal Blocking Operations


#define SIG_BLOCK          0    /* for blocking signals */
#define SIG_UNBLOCK        1    /* for unblocking signals */
#define SIG_SETMASK        2    /* for setting the signal mask */


These constants are used with `sigprocmask()` to specify how to modify the process signal mask.

## Default Signal Handlers


#define SIG_DFL        ((void (*)(int))0)    /* default signal handling */
#define SIG_IGN        ((void (*)(int))1)    /* ignore signal */
#define SIG_ERR        ((void (*)(int))-1)   /* error return from signal */


These special handler values are used to:
- **SIG_DFL**: Restore default handling for a signal
- **SIG_IGN**: Ignore a signal entirely
- **SIG_ERR**: Indicates an error return from signal-related functions

## Signal Action Structure


struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};


This structure configures the handling of a specific signal:

- **sa_handler**: Function pointer to the signal handler
- **sa_mask**: Additional signals to block during the handler execution
- **sa_flags**: Special flags that modify the behavior of the signal
- **sa_restorer**: Internal function used by the system to restore process context

## Function Prototypes

### Signal Handler Management


typedef void sigfunc(int);
sigfunc *signal(int signr, sigfunc *handler);

Sets a handler for a specific signal, returning the previous handler.


int sigaction(int sig, struct sigaction *act, struct sigaction *oldact);

More advanced interface for establishing signal handlers with additional options.

### Signal Generation


int raise(int sig);

Sends a signal to the current process.


int kill(pid_t pid, int sig);

Sends a signal to a specified process or process group.

### Signal Set Manipulation


int sigaddset(sigset_t *mask, int signo);

Adds a specific signal to a signal set.


int sigdelset(sigset_t *mask, int signo);

Removes a specific signal from a signal set.


int sigemptyset(sigset_t *mask);

Initializes a signal set to be empty (no signals).


int sigfillset(sigset_t *mask);

Initializes a signal set to include all signals.


int sigismember(sigset_t *mask, int signo);

Tests if a specific signal is in a signal set (returns 1 if present, 0 if not).

### Process Signal Mask Management


int sigpending(sigset_t *set);

Examines signals that are blocked and pending for the current process.


int sigprocmask(int how, sigset_t *set, sigset_t *oldset);

Changes the current process signal mask according to the 'how' parameter.


int sigsuspend(sigset_t *sigmask);

Temporarily replaces the signal mask and suspends the process until a signal is received.

## Implementation Notes

This header provides POSIX-compatible signal handling facilities, with some Linux-specific extensions. The implementation uses a simple bit-vector approach for representing signal sets, where each of the 32 bits in a `sigset_t` corresponds to a specific signal number.

The commented-out macros for `sigemptyset()` and `sigfillset()` suggest that these functions are implemented in the C library rather than as inline macros, which offers better type checking and debugging capabilities at the cost of a small performance overhead.

