# Linux Floating-Point Math Emulation Header

## Overview

This header file (`math_emu.h`) defines the core structures and functions necessary for implementing floating-point math emulation in the Linux kernel. It provides software emulation for floating-point operations on systems that don't have a hardware floating-point unit (FPU) or when the FPU is disabled. Part of the original Linux kernel developed by Linus Torvalds in 1991, this code represents a critical component for maintaining mathematical computation capabilities across all hardware platforms.

## Architecture Overview

The math emulation subsystem intercepts floating-point instructions that would normally be executed by the FPU, handling them in software instead. This allows floating-point operations to continue working even without hardware support, maintaining compatibility at the cost of performance.

## Core Data Structures

### Processor State Information


struct info {
    long ___math_ret;     // Return value from math functions
    long ___orig_eip;     // Original instruction pointer when exception occurred
    long ___edi;          // Saved register values
    long ___esi;
    long ___ebp;
    long ___sys_call_ret; // System call return value
    long ___eax;          // General purpose registers
    long ___ebx;
    long ___ecx;
    long ___edx;
    long ___orig_eax;     // Original EAX value
    long ___fs;           // Segment registers
    long ___es;
    long ___ds;
    long ___eip;          // Current instruction pointer
    long ___cs;           // Code segment
    long ___eflags;       // Flags register
    long ___esp;          // Stack pointer
    long ___ss;           // Stack segment
};


This structure stores the complete processor state when a floating-point instruction is intercepted. It allows the emulator to access and modify registers as needed during emulation.

### Register Access Macros

The header defines macros to provide clean access to the registers saved in the `info` structure:


#define EAX (info->___eax)
#define EBX (info->___ebx)
// ... and so on for all registers


These macros make the code more readable by providing familiar x86 register names instead of structure member access syntax.

### Floating-Point Number Representations

The emulator supports multiple floating-point formats:


typedef struct {
    long a,b;
    short exponent;
} temp_real;

typedef struct {
    short m0,m1,m2,m3;
    short exponent;
} temp_real_unaligned;

typedef struct {
    long a,b;
} long_real;

typedef long short_real;

typedef struct {
    long a,b;
    short sign;
} temp_int;


- **temp_real**: 80-bit extended precision format (x87 internal format)
- **temp_real_unaligned**: Alternative representation of the 80-bit format
- **long_real**: 64-bit double precision format
- **short_real**: 32-bit single precision format
- **temp_int**: Representation for integer values during conversions

### Status Word Structure


struct swd {
    int ie:1;    // Invalid operation exception
    int de:1;    // Denormalized operand exception
    int ze:1;    // Zero divide exception
    int oe:1;    // Overflow exception
    int ue:1;    // Underflow exception
    int pe:1;    // Precision exception
    int sf:1;    // Stack fault
    int ir:1;    // Interrupt requested
    int c0:1;    // Condition code bit 0
    int c1:1;    // Condition code bit 1
    int c2:1;    // Condition code bit 2
    int top:3;   // Stack top pointer (0-7)
    int c3:1;    // Condition code bit 3
    int b:1;     // Busy flag
};


This structure mirrors the FPU status word, which contains information about the current state of the FPU, including exception flags and condition codes.

## Constants and Configuration

### Common Mathematical Constants

The header provides predefined mathematical constants in the extended precision format:


#define CONSTZ   (temp_real_unaligned) {0x0000,0x0000,0x0000,0x0000,0x0000}    // Zero
#define CONST1   (temp_real_unaligned) {0x0000,0x0000,0x0000,0x8000,0x3FFF}    // One
#define CONSTPI  (temp_real_unaligned) {0xC235,0x2168,0xDAA2,0xC90F,0x4000}    // Pi
#define CONSTLN2 (temp_real_unaligned) {0x79AC,0xD1CF,0x17F7,0xB172,0x3FFE}    // Natural log of 2
#define CONSTLG2 (temp_real_unaligned) {0xF799,0xFBCF,0x9A84,0x9A20,0x3FFD}    // Log base 10 of 2
#define CONSTL2E (temp_real_unaligned) {0xF0BC,0x5C17,0x3B29,0xB8AA,0x3FFF}    // Log base 2 of e
#define CONSTL2T (temp_real_unaligned) {0x8AFE,0xCD1B,0x784B,0xD49A,0x4000}    // Log base 2 of 10


These constants eliminate the need to calculate common values repeatedly.

### Rounding and Precision Modes

The emulator supports different rounding modes and precision levels that match the x87 FPU capabilities:


// Precision control
#define BITS24	0    // Single precision (24 bits)
#define BITS53	2    // Double precision (53 bits)
#define BITS64	3    // Extended precision (64 bits)

// Rounding control
#define ROUND_NEAREST	0    // Round to nearest even
#define ROUND_DOWN	1    // Round toward -∞
#define ROUND_UP	2    // Round toward +∞
#define ROUND_0		3    // Round toward zero (truncate)


### Exception Flag Setting Macros

The header provides macros for setting different exception flags in the status word:


#define set_IE() (I387.swd |= 1)      // Set Invalid Operation Exception
#define set_DE() (I387.swd |= 2)      // Set Denormal Operand Exception
#define set_ZE() (I387.swd |= 4)      // Set Zero Divide Exception
#define set_OE() (I387.swd |= 8)      // Set Overflow Exception
#define set_UE() (I387.swd |= 16)     // Set Underflow Exception
#define set_PE() (I387.swd |= 32)     // Set Precision Exception


## Function Declarations

The header declares several categories of functions needed for emulation:

### Error Handling


void __math_abort(struct info *, unsigned int);
#define math_abort(x,y) \
(((void (*)(struct info *,unsigned int)) __math_abort)((x),(y)))

These functions handle errors that occur during emulation, such as invalid operations or unsupported features.

### Operand Fetch and Store

Functions for loading and storing operands in different formats:


void get_short_real(temp_real *, struct info *, unsigned short);    // Load 32-bit float
void get_long_real(temp_real *, struct info *, unsigned short);     // Load 64-bit double
void get_temp_real(temp_real *, struct info *, unsigned short);     // Load 80-bit extended
// ... more functions for different formats


### Format Conversion

Functions for converting between different numeric representations:


void short_to_temp(const short_real * __a, temp_real * __b);    // Convert 32-bit to extended
void long_to_temp(const long_real * __a, temp_real * __b);      // Convert 64-bit to extended
void temp_to_short(const temp_real * __a, short_real * __b);    // Convert extended to 32-bit
// ... more conversion functions


### Arithmetic Operations

Core mathematical operations implemented in software:


void fadd(const temp_real *, const temp_real *, temp_real *);    // Addition
void fmul(const temp_real *, const temp_real *, temp_real *);    // Multiplication
void fdiv(const temp_real *, const temp_real *, temp_real *);    // Division


### Comparison Operations

Functions for comparing floating-point values:


void fcom(const temp_real *, const temp_real *);    // Compare floating-point values
void fucom(const temp_real *, const temp_real *);   // Unordered compare
void ftst(const temp_real *);                       // Test against zero


### Instruction Decoding


char * ea(struct info * __info, unsigned short __code);    // Calculate effective address


This function decodes x86 addressing modes and calculates the effective memory address for operands.

## Technical Implementation Notes

### Alignment Challenges

The header includes a comment about an alignment issue with the `temp_real` structure:


/*
 * Gcc forces this stupid alignment problem: I want to use only two longs
 * for the temporary real 64-bit mantissa, but then gcc aligns out the
 * structure to 12 bytes which breaks things in math_emulate.c. Shit. I
 * want some kind of "no-alignt" pragma or something.
 */


This issue led to the creation of the separate `temp_real_unaligned` structure to work around compiler-forced alignment that would break the emulation code.

### Memory and Performance Considerations

The math emulation code prioritizes correctness over performance, as it's only used when hardware FPU is unavailable. The careful bit-by-bit implementation ensures accurate results at the cost of significantly slower execution compared to hardware.

## Conclusion

The Linux math emulation subsystem represented by this header file is a critical fallback mechanism that ensures mathematical operations work consistently across all hardware platforms. While most modern systems have hardware floating-point units, this code remains important for compatibility with older hardware and for handling edge cases where the FPU might be disabled or unavailable.

This emulation layer demonstrates the Linux kernel's commitment to maintaining compatibility and functionality across diverse hardware environments, even when it requires implementing complex mathematical operations in software.

