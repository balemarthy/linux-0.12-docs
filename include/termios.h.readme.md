# Linux Terminal I/O Control Interface (`termios.h`)

## Overview

This header file defines the Linux terminal I/O interface structures and constants that allow programs to control terminal devices. The `termios` interface provides a way to configure terminal parameters such as baud rate, character size, flow control, and special character handling. This is essential for any program that needs to interact with terminal devices directly, such as terminal emulators, serial communication programs, or interactive command-line applications.

## Key Structures

### Window Size Structure


struct winsize {
    unsigned short ws_row;     /* Number of rows (in characters) */
    unsigned short ws_col;     /* Number of columns (in characters) */
    unsigned short ws_xpixel;  /* Width in pixels */
    unsigned short ws_ypixel;  /* Height in pixels */
};


This structure holds information about the terminal window dimensions, which is crucial for applications that need to format output according to the current terminal size.

### Terminal I/O Structure (Legacy)


#define NCC 8
struct termio {
    unsigned short c_iflag;    /* Input mode flags */
    unsigned short c_oflag;    /* Output mode flags */
    unsigned short c_cflag;    /* Control mode flags */
    unsigned short c_lflag;    /* Local mode flags */
    unsigned char c_line;      /* Line discipline */
    unsigned char c_cc[NCC];   /* Control characters */
};


The `termio` structure is a legacy interface that predates POSIX standards. Modern applications should use the POSIX-compliant `termios` structure instead.

### POSIX Terminal I/O Structure


#define NCCS 17
struct termios {
    tcflag_t c_iflag;          /* Input mode flags */
    tcflag_t c_oflag;          /* Output mode flags */
    tcflag_t c_cflag;          /* Control mode flags */
    tcflag_t c_lflag;          /* Local mode flags */
    cc_t c_line;               /* Line discipline */
    cc_t c_cc[NCCS];           /* Control characters */
};


This is the primary structure used to configure terminal settings in POSIX-compliant systems. Each field controls a different aspect of terminal behavior:

- **c_iflag**: Controls input processing (character conversion, flow control)
- **c_oflag**: Controls output processing (character conversion, newline handling)
- **c_cflag**: Controls serial communication parameters (baud rate, parity)
- **c_lflag**: Controls terminal behavior ("cooked" mode, echoing)
- **c_line**: Selects the line discipline (rarely used in modern systems)
- **c_cc**: Array of special control characters (interrupt, erase, etc.)

## Terminal Control Characters

The `c_cc` array in the `termios` structure contains special control characters that perform specific functions:

| Index | Symbolic Name | Description |
|-------|--------------|-------------|
| 0 | VINTR | Interrupt character (typically Ctrl+C) |
| 1 | VQUIT | Quit character (typically Ctrl+\\) |
| 2 | VERASE | Erase character (typically Backspace) |
| 3 | VKILL | Kill line character (typically Ctrl+U) |
| 4 | VEOF | End-of-file character (typically Ctrl+D) |
| 5 | VTIME | Used for timing in non-canonical mode |
| 6 | VMIN | Minimum number of characters for reads |
| 7 | VSWTC | Switch character (rarely used) |
| 8 | VSTART | Start character for flow control (typically Ctrl+Q) |
| 9 | VSTOP | Stop character for flow control (typically Ctrl+S) |
| 10 | VSUSP | Suspend character (typically Ctrl+Z) |
| 11 | VEOL | End-of-line character |
| 12 | VREPRINT | Reprint character (typically Ctrl+R) |
| 13 | VDISCARD | Discard character (typically Ctrl+O) |
| 14 | VWERASE | Word erase character (typically Ctrl+W) |
| 15 | VLNEXT | Literal next character (typically Ctrl+V) |
| 16 | VEOL2 | Second end-of-line character |

## Flag Definitions

### Input Mode Flags (`c_iflag`)

These flags control how input characters are processed:


/* c_iflag bits */
#define IGNBRK  0000001  /* Ignore break condition */
#define BRKINT  0000002  /* Map break to SIGINT */
#define IGNPAR  0000004  /* Ignore characters with parity errors */
#define PARMRK  0000010  /* Mark parity errors */
#define INPCK   0000020  /* Enable input parity checking */
#define ISTRIP  0000040  /* Strip high bit from input characters */
#define INLCR   0000100  /* Translate NL to CR on input */
#define IGNCR   0000200  /* Ignore CR */
#define ICRNL   0000400  /* Translate CR to NL on input */
#define IUCLC   0001000  /* Map uppercase to lowercase on input */
#define IXON    0002000  /* Enable start/stop output control */
#define IXANY   0004000  /* Enable any character to restart output */
#define IXOFF   0010000  /* Enable start/stop input control */
#define IMAXBEL 0020000  /* Ring bell when input queue is full */


### Output Mode Flags (`c_oflag`)

These flags control how output characters are processed:


/* c_oflag bits */
#define OPOST   0000001  /* Perform output processing */
#define OLCUC   0000002  /* Map lowercase to uppercase on output */
#define ONLCR   0000004  /* Map NL to CR-NL on output */
#define OCRNL   0000010  /* Map CR to NL on output */
#define ONOCR   0000020  /* Don't output CR at column 0 */
#define ONLRET  0000040  /* Don't output CR */
#define OFILL   0000100  /* Use fill characters for delay */
#define OFDEL   0000200  /* Fill is DEL, else NUL */


The header also defines various delay constants for newlines, carriage returns, tabs, etc.

### Control Mode Flags (`c_cflag`)

These flags control the hardware characteristics of the terminal:


/* c_cflag bit meaning */
#define CBAUD   0000017  /* Baud rate mask */
/* Baud rate constants */
#define  B0     0000000  /* Hang up */
#define  B50    0000001  /* 50 baud */
#define  B75    0000002  /* 75 baud */
/* ... more baud rates ... */
#define  B38400 0000017  /* 38400 baud */

#define CSIZE   0000060  /* Character size mask */
#define   CS5   0000000  /* 5 bits */
#define   CS6   0000020  /* 6 bits */
#define   CS7   0000040  /* 7 bits */
#define   CS8   0000060  /* 8 bits */

#define CSTOPB  0000100  /* 2 stop bits (else 1) */
#define CREAD   0000200  /* Enable receiver */
#define PARENB  0000400  /* Parity enable */
#define PARODD  0001000  /* Odd parity (else even) */
#define HUPCL   0002000  /* Hang up on last close */
#define CLOCAL  0004000  /* Ignore modem control lines */
#define CRTSCTS 020000000000  /* RTS/CTS flow control */


### Local Mode Flags (`c_lflag`)

These flags control the terminal's behavior:


/* c_lflag bits */
#define ISIG    0000001  /* Enable signals */
#define ICANON  0000002  /* Canonical input (erase and kill processing) */
#define XCASE   0000004  /* Canonical upper/lower presentation */
#define ECHO    0000010  /* Enable echo */
#define ECHOE   0000020  /* Echo erase character as BS-SP-BS */
#define ECHOK   0000040  /* Echo NL after kill character */
#define ECHONL  0000100  /* Echo NL */
#define NOFLSH  0000200  /* Disable flush after interrupt or quit */
#define TOSTOP  0000400  /* Send SIGTTOU for background output */
#define ECHOCTL 0001000  /* Echo control characters as ^X */
#define ECHOPRT 0002000  /* Echo erase as character erased */
#define ECHOKE  0004000  /* BS-SP-BS erase entire line on line kill */
#define FLUSHO  0010000  /* Output being flushed */
#define PENDIN  0040000  /* Retype pending input at next read */
#define IEXTEN  0100000  /* Enable extended input processing */


## Terminal I/O Control Requests

The header defines numerous `ioctl` request codes for terminal operations:


/* 0x54 is just a magic number to make these relatively uniqe ('T') */
#define TCGETS      0x5401  /* Get termios struct */
#define TCSETS      0x5402  /* Set termios struct */
#define TCSETSW     0x5403  /* Set termios struct, drain output first */
#define TCSETSF     0x5404  /* Set termios struct, flush I/O first */
/* ... more ioctl codes ... */


These codes are used with the `ioctl()` system call to perform various operations on terminals.

## Modem Control Lines

The header defines constants for

