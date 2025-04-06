# Linux Kernel Memory Access Functions

## Overview

This file contains low-level functions for accessing memory across segment boundaries in the Linux kernel. These functions allow the kernel to safely read from and write to user space memory using the x86 segmentation feature via the FS segment register.

## Background

In x86 architecture, segment registers (CS, DS, ES, FS, GS, SS) are used to define memory segments. The Linux kernel uses the FS register to access user-space memory from within kernel mode. This segmentation mechanism provides hardware-enforced memory protection between kernel and user space.

## Memory Access Functions

### Reading from User Space

These functions retrieve data of different sizes from user space memory:


static inline unsigned char get_fs_byte(const char * addr)
{
    unsigned register char _v;

    __asm__ ("movb %%fs:%1,%0":"=q" (_v):"m" (*addr));
    return _v;
}


**What**: Reads a single byte from user space memory.  
**How**: Uses inline assembly with the FS segment prefix to access the byte at the specified address.  
**Why**: Direct register-level access provides efficient and controlled boundary crossing.


static inline unsigned short get_fs_word(const unsigned short *addr)
{
    unsigned short _v;

    __asm__ ("movw %%fs:%1,%0":"=q" (_v):"m" (*addr));
    return _v;
}


**What**: Reads a 16-bit word from user space memory.  
**How**: Similar to `get_fs_byte`, but uses `movw` instruction for word-sized access.  
**Why**: Provides size-specific optimized access for 16-bit values.


static inline unsigned long get_fs_long(const unsigned long *addr)
{
    unsigned long _v;

    __asm__ ("movl %%fs:%1,%0":"=q" (_v):"m" (*addr));
    return _v;
}


**What**: Reads a 32-bit long word from user space memory.  
**How**: Uses the `movl` instruction to read a 32-bit value through the FS segment.  
**Why**: Completes the set of size-specific memory access functions for efficient data transfer.

### Writing to User Space

These functions write data of different sizes to user space memory:


static inline void put_fs_byte(char val, char *addr)
{
    __asm__ ("movb %0,%%fs:%1"::"q" (val),"m" (*addr));
}


**What**: Writes a single byte to user space memory.  
**How**: Uses inline assembly to move the value to the memory location specified by addr, accessed through the FS segment.  
**Why**: Provides controlled write access to user space with minimal overhead.


static inline void put_fs_word(short val, short * addr)
{
    __asm__ ("movw %0,%%fs:%1"::"q" (val),"m" (*addr));
}


**What**: Writes a 16-bit word to user space memory.  
**How**: Uses the `movw` instruction to write a word-sized value through the FS segment.  
**Why**: Size-specific optimized write access for 16-bit values.


static inline void put_fs_long(unsigned long val, unsigned long * addr)
{
    __asm__ ("movl %0,%%fs:%1"::"q" (val),"m" (*addr));
}


**What**: Writes a 32-bit long word to user space memory.  
**How**: Uses the `movl` instruction to write a 32-bit value through the FS segment.  
**Why**: Completes the write function set for comprehensive memory interaction capabilities.

## Segment Register Management

These functions allow for getting and setting segment register values:


static inline unsigned long get_fs() 
{
    unsigned short _v;
    __asm__("mov %%fs,%%ax":"=a" (_v):);
    return _v;
}


**What**: Retrieves the current value of the FS segment register.  
**How**: Moves the FS register value into the AX register, then returns it.  
**Why**: Allows the kernel to inspect and preserve the current segmentation context.


static inline unsigned long get_ds() 
{
    unsigned short _v;
    __asm__("mov %%ds,%%ax":"=a" (_v):);
    return _v;
}


**What**: Retrieves the current value of the DS (data segment) register.  
**How**: Similar to `get_fs()`, but reads from the DS register instead.  
**Why**: Provides access to the data segment value for context management.


static inline void set_fs(unsigned long val)
{
    __asm__("mov %0,%%fs"::"a" ((unsigned short) val));
}


**What**: Sets the FS segment register to a specified value.  
**How**: Moves the provided value from the AX register into the FS register.  
**Why**: Allows the kernel to switch between different memory contexts, particularly useful when transitioning between kernel and user space operations.

## Technical Implementation Details

### GCC Extended Asm Syntax

The functions use GCC's extended assembly syntax with this format:


asm("assembly code" : output operands : input operands : clobbered registers);


For example, in `get_fs_byte`:
- `"movb %%fs:%1,%0"` is the assembly instruction
- `"=q" (_v)` is the output operand (result goes into _v)
- `"m" (*addr)` is the input operand (memory operand at *addr)

The `%%` prefix is needed because GCC uses `%` for operands, so `%%` outputs a literal `%` in the assembly.

### Segment Addressing

The `%fs:` prefix in assembly instructions causes the CPU to use the FS segment register as a base for the memory access. This is a key x86 architecture feature that allows the kernel to safely access user space memory in a controlled manner.

### Constraints

- `=q` indicates the output should be in a byte-addressable register
- `=a` specifically requests the AX register
- `m` indicates a memory operand

## Historical Note

The original comment in the code (removed as requested) indicates these functions were written in the early days of Linux kernel development (circa 1991) and validated by Linus Torvalds himself, highlighting the critical and foundational nature of these low-level memory access functions.

