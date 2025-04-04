# Linux Terminal I/O Subsystem (tty.h)

## Overview

This header file defines the core structures and constants for the Linux terminal I/O subsystem. The `tty.h` file is a fundamental component that manages the interaction between user terminals, serial devices, and pseudoterminals in the Linux kernel. It provides the foundation for all terminal communication within the operating system.

## Terminal Types and Device Management

The system supports multiple types of terminal devices:


#define MAX_CONSOLES   8
#define NR_SERIALS     2
#define NR_PTYS        4

extern int NR_CONSOLES;


These constants define the maximum number of supported terminals:
- Up to 8 console terminals (physical display/keyboard combinations)
- 2 serial ports for external terminal devices
- 4 pseudoterminal pairs (used for terminal emulation)

The actual number of consoles in use is stored in `NR_CONSOLES`, which can be dynamically set at boot time.

## Terminal Queue Implementation

The core data structure for handling input and output is the `tty_queue`:


#define TTY_BUF_SIZE 1024

struct tty_queue {
    unsigned long data;
    unsigned long head;
    unsigned long tail;
    struct task_struct * proc_list;
    char buf[TTY_BUF_SIZE];
};


This implements a circular buffer with:
- A fixed size buffer of 1024 bytes for storing characters
- Head and tail pointers to manage the FIFO queue
- A linked list of processes waiting on this queue
- An additional data field for queue-specific information

The circular buffer design is critical for efficient terminal I/O as it allows:
1. Non-blocking writes when the terminal is slower than the program
2. Buffering of keyboard input for later processing
3. Managing flow control in a memory-efficient way

## Queue Manipulation Macros

A set of macros make working with the circular queue more convenient:


#define INC(a) ((a) = ((a)+1) & (TTY_BUF_SIZE-1))
#define DEC(a) ((a) = ((a)-1) & (TTY_BUF_SIZE-1))
#define EMPTY(a) ((a)->head == (a)->tail)
#define LEFT(a) (((a)->tail-(a)->head-1)&(TTY_BUF_SIZE-1))
#define LAST(a) ((a)->buf[(TTY_BUF_SIZE-1)&((a)->head-1)])
#define FULL(a) (!LEFT(a))
#define CHARS(a) (((a)->head-(a)->tail)&(TTY_BUF_SIZE-1))
#define GETCH(queue,c) \
(void)({c=(queue)->buf[(queue)->tail];INC((queue)->tail);})
#define PUTCH(c,queue) \
(void)({(queue)->buf[(queue)->head]=(c);INC((queue)->head);})


These macros allow:
- **INC/DEC**: Increment/decrement pointers with automatic wrap-around
- **EMPTY/FULL**: Quick buffer state checking
- **LEFT/CHARS**: Determine space left or characters available
- **LAST**: Examine the last character added
- **GETCH/PUTCH**: Add or remove characters atomically

The bitwise AND with `(TTY_BUF_SIZE-1)` is an optimization that works because the buffer size is a power of 2, making modulo operations faster.

## Device Type Identification

The system uses bit patterns in the minor device number to identify terminal types:


#define IS_A_CONSOLE(min)     (((min) & 0xC0) == 0x00)
#define IS_A_SERIAL(min)      (((min) & 0xC0) == 0x40)
#define IS_A_PTY(min)         ((min) & 0x80)
#define IS_A_PTY_MASTER(min)  (((min) & 0xC0) == 0x80)
#define IS_A_PTY_SLAVE(min)   (((min) & 0xC0) == 0xC0)
#define PTY_OTHER(min)        ((min) ^ 0x40)


This bit-masking approach efficiently categorizes devices:
- Consoles use minor numbers 0-63
- Serial devices use minor numbers 64-127
- PTY masters use minor numbers 128-191
- PTY slaves use minor numbers 192-255

The `PTY_OTHER` macro cleverly toggles between master and slave devices of a pseudoterminal pair by flipping a single bit.

## Terminal Control Characters

Special terminal control characters are defined as macros for easy access:


#define INTR_CHAR(tty) ((tty)->termios.c_cc[VINTR])
#define QUIT_CHAR(tty) ((tty)->termios.c_cc[VQUIT])
#define ERASE_CHAR(tty) ((tty)->termios.c_cc[VERASE])
#define KILL_CHAR(tty) ((tty)->termios.c_cc[VKILL])
#define EOF_CHAR(tty) ((tty)->termios.c_cc[VEOF])
#define START_CHAR(tty) ((tty)->termios.c_cc[VSTART])
#define STOP_CHAR(tty) ((tty)->termios.c_cc[VSTOP])
#define SUSPEND_CHAR(tty) ((tty)->termios.c_cc[VSUSP])


These macros provide easy access to control characters like:
- **INTR_CHAR**: Interrupt character (typically Ctrl+C)
- **QUIT_CHAR**: Quit character (typically Ctrl+\)
- **ERASE_CHAR**: Character delete (backspace/delete)
- **KILL_CHAR**: Line kill character (typically Ctrl+U)
- **EOF_CHAR**: End-of-file marker (typically Ctrl+D)
- **START/STOP_CHAR**: Flow control characters (Ctrl+Q/Ctrl+S)
- **SUSPEND_CHAR**: Process suspension (typically Ctrl+Z)

## Terminal Structure

The main terminal structure binds everything together:


struct tty_struct {
    struct termios termios;
    int pgrp;
    int session;
    int stopped;
    void (*write)(struct tty_struct * tty);
    struct tty_queue *read_q;
    struct tty_queue *write_q;
    struct tty_queue *secondary;
};


This structure encapsulates:
- **termios**: POSIX terminal settings (baud rate, control chars, mode flags)
- **pgrp/session**: Process group and session for job control
- **stopped**: Flow control state flag
- **write**: Function pointer for device-specific output handling
- **Three queues**:
  - **read_q**: Raw input from the device
  - **write_q**: Output waiting to be sent to the device
  - **secondary**: Processed input ready for programs to read

## Terminal Access and Initialization

Functions and globals for managing terminals:


extern struct tty_struct tty_table[];
extern int fg_console;

#define TTY_TABLE(nr) \
(tty_table + ((nr) ? (((nr) < 64)? (nr)-1:(nr)) : fg_console))

#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"

void rs_init(void);
void con_init(void);
void tty_init(void);


Key components:
- **tty_table[]**: Array of all terminal structures
- **fg_console**: Currently active console
- **TTY_TABLE** macro: Converts device numbers to array indices
- **INIT_C_CC**: Default control character values
- Initialization functions for different device types

## I/O and Processing Functions

The interface between the terminal subsystem and the rest of the kernel:


int tty_read(unsigned c, char * buf, int n);
int tty_write(unsigned c, char * buf, int n);

void con_write(struct tty_struct * tty);
void rs_write(struct tty_struct * tty);
void mpty_write(struct tty_struct * tty);
void spty_write(struct tty_struct * tty);

void copy_to_cooked(struct tty_struct * tty);
void update_screen(void);


These functions handle:
- **tty_read/tty_write**: File operation handlers for terminal devices
- **Device-specific write functions**: Implementations for different terminal types
- **copy_to_cooked**: Line discipline processing (raw to cooked mode conversion)
- **update_screen**: Console display refreshing

## Engineering Significance

This implementation demonstrates several important systems programming principles:

1. **Separation of concerns** between generic terminal handling and device-specific operations
2. **Efficient buffer management** with circular queues
3. **Clever bit manipulation** for device type identification
4. **Function pointer polymorphism** for device-specific operations
5. **POSIX compliance** through the termios structure
6. **Memory efficiency** through fixed buffer sizes and shared code paths

    The terminal subsystem is one of the most critical interfaces between user applications and the kernel, making its design particularly important for system usability, reliability, and performance.

