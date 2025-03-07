# Detailed Documentation for `bootsect.asm`

This file contains the boot sector code for the Linux kernel. It is responsible for loading the kernel into memory and starting its execution.

## Overview

The `bootsect.asm` file is divided into several sections:

1. **Equates and Constants**: Defines various constants used throughout the code.
2. **Relocation and Jump**: Moves the boot sector code to a new memory location and jumps to it.
3. **Segment Setup**: Sets up the necessary segment registers.
4. **Disk Parameter Table Setup**: Creates and patches the disk parameter table.
5. **Floppy Disk Controller Reset**: Resets the floppy disk controller.
6. **Loading Setup Sectors**: Loads the setup sectors directly after the boot block.
7. **Loading the Kernel**: Loads the kernel into memory.
8. **Root Device Selection**: Determines which root device to use.
9. **Read and Print Functions**: Contains functions for reading from disk and printing debug information.
10. **Floppy Drive Motor Control**: Contains a function to turn off the floppy drive motor.

## Detailed Explanation

### Equates and Constants

/*!
 * @brief Defines the number of setup sectors.
 */

SETUPLEN equ 4

/*!
 * @brief Defines the segment address for the boot sector.
 */

BOOTSEG equ 0x07c0

/*!
 * @brief Defines the segment address for the initialization code.
 */

INITSEG equ 0x9000

/*!
 * @brief Defines the segment address for the setup code.
 */

SETUPSEG equ 0x9020

/*!
 * @brief Defines the segment address for the system code.
 */

SYSSEG equ 0x1000

/*!
 * @brief Defines the size of the system code in segments.
 */

SYSSIZE equ 0x3000

/*!
 * @brief Defines the end segment address for the system code.
 */

ENDSEG equ SYSSEG+SYSSIZE

/*!
 * @brief Defines the root device identifier.
 */

ROOT_DEV equ 0x0301

/*!
 * @brief Defines the swap device identifier.
 */

SWAP_DEV equ 0x0304


### Relocation and Jump

/*!
 * @brief Moves the boot sector code to a new memory location and jumps to it.
 * 
 * This section of code moves the boot sector code from its initial loading
 * address at 0x7c00 to the new initialization segment at 0x90000. This is
 * done to free up the initial memory area for other uses.
 */

start:
    mov ax, BOOTSEG /*!< Load the original boot sector segment address into AX */
    mov ds, ax /*!< Set DS to the original boot sector segment */
    mov ax, INITSEG /*!< Load the new segment address into AX */
    mov es, ax /*!< Set ES to the new segment address */
    mov cx, 256 /*!< Set CX to 256, representing the number of words (512 bytes) to copy */
    sub si, si /*!< Set SI to 0, the source index */
    sub di, di /*!< Set DI to 0, the destination index */
    rep movsw /*!< Copy 512 bytes from DS:SI to ES:DI */
    jmp INITSEG:go /*!< Jump to the 'go' label in the new segment */


### Segment Setup

/*!
 * @brief Sets up the necessary segment registers.
 * 
 * The 'go' label marks the start of the code in the new segment. This section
 * sets up the segment registers and the stack pointer.
 */
go:
    mov ax, cs /*!< Copy CS to AX */
    mov dx, 0xfef4 /*!< Load an arbitrary value into DX */
    mov ds, ax /*!< Set DS to the value in AX */
    mov es, ax /*!< Set ES to the value in AX */
    mov ss, ax /*!< Set SS to the value in AX */
    mov sp, dx /*!< Set SP to the value in DX */

### Disk Parameter Table Setup

/*!
 * @brief Creates and patches the disk parameter table.
 * 
 * This section sets up new disk parameter tables in RAM for multi-sector reads.
 * It then patches the sector count in the parameter table.
 */
xor ax, ax /*!< Clear AX */
mov fs, ax /*!< Set FS to 0 */
lds si, [fs:0x78] /*!< Load the source index from the parameter table address */
mov di, dx /*!< Set DI to the value in DX */
mov cx, 6 /*!< Set CX to 6, representing the number of words (12 bytes) to copy */
cld /*!< Clear the direction flag */
rep movsw /*!< Copy 12 bytes from FS:SI to ES:DI */

mov ax, cs /*!< Copy CS to AX */
mov ds, ax /*!< Set DS to the value in AX */
mov di, dx /*!< Set DI to the value in DX */
mov byte [es:di+4], 18 /*!< Patch the sector count at offset 4 in DI */

mov [fs:bx], di /*!< Store DI at FS:BX */
mov [fs:bx+2], es /*!< Store ES at FS:BX+2 */

mov fs, ax /*!< Set FS to the value in AX */
mov gs, ax /*!< Set GS to the value in AX */

### Floppy Disk Controller Reset

/*!
 * @brief Resets the floppy disk controller.
 * 
 * This section resets the floppy disk controller using BIOS interrupts.
 */
xor ah, ah /*!< Clear AH */
xor dl, dl /*!< Clear DL */
int 0x13 /*!< Call interrupt 0x13 to reset the disk controller */

### Loading Setup Sectors

/*!
 * @brief Loads the setup sectors directly after the boot block.
 * 
 * This section loads the setup sectors directly after the boot block using BIOS interrupts.
 */
load_setup:
    xor dx, dx /*!< Clear DX */
    mov cx, 0x0002 /*!< Set CX to sector 2, track 0 */
    mov bx, 0x0200 /*!< Set BX to 512, the address in INITSEG */
    mov ax, 0x0200+SETUPLEN /*!< Set AX to service 2, number of sectors */
    int 0x13 /*!< Call interrupt 0x13 to read the sectors */
    jnc ok_load_setup /*!< Jump if no carry (no error) */

/*!
 * @brief Handles errors during sector loading.
 * 
 * If an error occurs during sector loading, the error code is pushed onto
 * the stack and printed using 'print_nl' and 'print_hex'. The disk controller
 * is then reset and the operation retried.
 */
push ax /*!< Push AX onto the stack */
call print_nl /*!< Call print_nl */
mov bp, sp /*!< Set BP to SP */
call print_hex /*!< Call print_hex */
pop ax /*!< Pop the stack value into AX */

xor dl, dl /*!< Clear DL */
xor ah, ah /*!< Clear AH */
int 0x13 /*!< Call interrupt 0x13 */
jmp load_setup /*!< Jump to load_setup */

ok_load_setup:
    xor dl, dl /*!< Clear DL */
    mov ah, 0x08 /*!< Set AH to 8, get drive parameters */
    int 0x13 /*!< Call interrupt 0x13 */
    xor ch, ch /*!< Clear CH */
    mov [sectors], cx /*!< Move CX to sectors */
    mov ax, INITSEG /*!< Set AX to INITSEG */
    mov es, ax /*!< Set ES to AX */

### Loading the Kernel

/*!
 * @brief Loads the kernel into memory.
 * 
 * This section loads the kernel into memory and prepares for execution.
 */

mov ah, 0x03 /*!< Set AH to 3, read cursor position */
xor bh, bh /*!< Clear BH */
int 0x10 /*!< Call interrupt 0x10 */

mov cx, 9 /*!< Set CX to 9 */
mov bx, 0x7 /*!< Set BX to page 0, attribute 7 (normal) */
mov bp, msg1 /*!< Set BP to msg1 */
mov ax, 0x1301 /*!< Set AX to write string, move cursor */
int 0x10 /*!< Call interrupt 0x10 */

mov ax, SYSSEG /*!< Set AX to SYSSEG */
mov es, ax /*!< Set ES to AX */
call read_it /*!< Call read_it */
call kill_motor /*!< Call kill_motor */
call print_nl /*!< Call print_nl */

### Root Device Selection

/*!
 * @brief Determines which root device to use.
 * 
 * This section determines the root device to use based on the number of sectors per track.
 */
mov ax,[root_dev] /*!< Move root_dev into AX */
or ax,ax /*!< OR AX with AX */
jne root_defined /*!< Jump if not equal to root_defined */

mov bx, [sectors] /*!< Move sectors into BX */
mov ax, 0x0208 /*!< Set AX to /dev/ps0 - 1.2Mb */
cmp bx, 15 /*!< Compare BX with 15 */
je root_defined /*!< Jump if equal to root_defined */
mov ax, 0x021c /*!< Set AX to /dev/PS0 - 1.44Mb */
cmp bx, 18 /*!< Compare BX with 18 */
je root_defined

undef_root:
jmp undef_root /*!< Jump to undef_root */
root_defined:
mov [root_dev], ax /*!< Move AX to root_dev */

jmp SETUPSEG:0 /*!< Jump to 0 in SETUPSEG */


### Loading the Kernel

/*!
 * @brief Loads the kernel into memory.
 * 
 * This section loads the kernel into memory and prepares for execution.
 */
mov ax, SYSSEG /*!< Set AX to SYSSEG */
mov es, ax /*!< Set ES to AX */
call read_it /*!< Call read_it */
call kill_motor /*!< Call kill_motor */
call print_nl /*!< Call print_nl */


### Root Device Selection

/*!
 * @brief Determines which root device to use.
 * 
 * This section determines the root device to use based on the number of sectors per track.
 */
mov ax,[root_dev] /*!< Move root_dev into AX */
or ax,ax /*!< OR AX with AX */
jne root_defined /*!< Jump if not equal to root_defined */

mov bx, [sectors] /*!< Move sectors into BX */
mov ax, 0x0208 /*!< Set AX to /dev/ps0 - 1.2Mb */
cmp bx, 15 /*!< Compare BX with 15 */
je root_defined /*!< Jump if equal to root_defined */
mov ax, 0x021c /*!< Set AX to /dev/PS0 - 1.44Mb */
cmp bx, 18 /*!< Compare BX with 18 */
je root_defined
undef_root:
jmp undef_root /*!< Jump to undef_root */
root_defined:
mov [root_dev], ax /*!< Move AX to root_dev */

jmp SETUPSEG:0 /*!< Jump to 0 in SETUPSEG */


### Reading the Kernel


/*!
 * @brief Reads the system at address 0x10000, ensuring no 64kB boundaries are crossed.
 * 
 * This routine loads the system at address 0x10000, making sure no 64kB boundaries are crossed.
 * It tries to load the system as fast as possible, loading whole tracks whenever possible.
 */
sread: dw 1+SETUPLEN /*!< Define sread initialized to 1+SETUPLEN */
head:  dw 0 /*!< Define head initialized to 0 */
track: dw 0 /*!< Define track initialized to 0 */

read_it:
    mov ax, es /*!< Move the value in es to ax */
    test ax, 0xfff /*!< Test if ax is at a 64kB boundary */
die:
    jne die /*!< Jump to die if es is not at a 64kB boundary */
    xor bx, bx /*!< Clear bx */

rp_read:
    mov ax, es /*!< Move the value in es to ax */
    cmp ax, ENDSEG /*!< Compare ax with ENDSEG */
    jb ok1_read /*!< Jump to ok1_read if ax is below ENDSEG */
    ret /*!< Return if ax is not below ENDSEG */

ok1_read:
    mov ax, [sectors] /*!< Move the value in sectors to ax */
    sub ax, [sread] /*!< Subtract sread from ax */
    mov cx, ax /*!< Move the result to cx */
    shl cx, 9 /*!< Shift cx left by 9 bits (multiply by 512) */
    add cx, bx /*!< Add bx to cx */
    jnc ok2_read /*!< Jump to ok2_read if there is no carry */
    je ok2_read /*!< Jump to ok2_read if cx is zero */

    xor ax, ax /*!< Clear ax */
    sub ax, bx /*!< Subtract bx from ax */
    shr ax, 9 /*!< Shift ax right by 9 bits (divide by 512) */

ok2_read:
    call read_track /*!< Call read_track */
    mov cx, ax /*!< Move the value in ax to cx */
    add ax, [sread] /*!< Add sread to ax */
    cmp ax, [sectors] /*!< Compare ax with sectors */
    jne ok3_read /*!< Jump to ok3_read if ax is not equal to sectors */

    mov ax, 1 /*!< Move 1 to ax */
    sub ax, [head] /*!< Subtract head from ax */
    jne ok4_read /*!< Jump to ok4_read if ax is not equal to head */
    inc word [track] /*!< Increment track */

ok4_read:
    mov [head], ax /*!< Move ax to head */
    xor ax, ax /*!< Clear ax */

ok3_read:
    mov [sread], ax /*!< Move ax to sread */
    shl cx, 9 /*!< Shift cx left by 9 bits (multiply by 512) */
    add bx, cx /*!< Add cx to bx */
    jnc rp_read /*!< Jump to rp_read if there is no carry */

    mov ax, es /*!< Move the value in es to ax */
    add ah, 0x10 /*!< Add 0x10 to ah */
    mov es, ax /*!< Move ax to es */
    xor bx, bx /*!< Clear bx */
    jmp rp_read /*!< Jump to rp_read */


### Reading a Track


/*!
 * @brief Reads a track from the disk.
 * 
 * This routine reads a track from the disk into memory.
 * It uses BIOS interrupts to perform the read operation.
 */
read_track:
    pusha /*!< Push all general-purpose registers onto the stack */
    pusha /*!< Push all general-purpose registers onto the stack again for extra safety */
    mov ax, 0xe2e /*!< Move 0xe2e (loading... message) to ax */
    mov bx, 7 /*!< Move 7 to bx */
    int 0x10 /*!< Call BIOS interrupt 0x10 to display the loading message */
    popa /*!< Pop all general-purpose registers from the stack */

    mov dx,[track] /*!< Move the value in track to dx */
    mov cx,[sread] /*!< Move the value in sread to cx */
    inc cx /*!< Increment cx */
    mov ch, dl /*!< Move the lower 8 bits of dx to the upper 8 bits of cx */
    mov dx, [head] /*!< Move the value in head to dx */
    mov dh, dl /*!< Move the lower 8 bits of dx to the upper 8 bits of dx */
    and dx, 0x0100 /*!< AND dx with 0x0100 to isolate the head bit */
    mov ah, 2 /*!< Move 2 to ah (BIOS read sectors function) */

    push dx /*!< Save dx on the stack for error handling */
    push cx /*!< Save cx on the stack for error handling */
    push bx /*!< Save bx on the stack for error handling */
    push ax /*!< Save ax on the stack for error handling */

    int 0x13 /*!< Call BIOS interrupt 0x13 to read the track */
    jc bad_rt /*!< Jump to bad_rt if there is a carry (error) */
    add sp, 8 /*!< Adjust the stack pointer to remove saved registers */
    popa /*!< Pop all general-purpose registers from the stack */
    ret /*!< Return from the function */

/*!
 * @brief Handles errors during track read.
 * 
 * If an error occurs during track read, the error code is saved and printed.
 * The routine then resets the disk controller and retries the read operation.
 */
bad_rt:
    push ax /*!< Save ax on the stack for error code */
    call print_all /*!< Call print_all to print the error and registers */

    xor ah, ah /*!< Clear ah */
    xor dl, dl /*!< Clear dl */
    int 0x13 /*!< Call BIOS interrupt 0x13 to reset the disk controller */

    add sp, 10 /*!< Adjust the stack pointer to remove saved registers and error code */
    popa /*!< Pop all general-purpose registers from the stack */
    jmp read_track /*!< Jump to read_track to retry the operation */


### Printing Functions


/*!
 * @brief Prints all registers.
 * 
 * This routine prints out all of the registers for debugging purposes.
 * It assumes the following stack frame:
 * - `dx`
 * - `cx`
 * - `bx`
 * - `ax`
 * - `error`
 * - `ret` <- sp
 */
print_all:
    mov cx, 5 /*!< Set cx to 5 (error code + 4 registers) */
    mov bp, sp /*!< Move the stack pointer to bp */

print_loop:
    push cx /*!< Save cx on the stack */
    call print_nl /*!< Call print_nl to print a newline for readability */
    jae no_reg /*!< Jump to no_reg if cx is non-negative (no register name needed) */

    mov ax, 0xe05 + 0x41 - 1 /*!< Move the ASCII value of 'A' to ax */
    sub al, cl /*!< Subtract cl from al to determine the register name */
    int 0x10 /*!< Call BIOS interrupt 0x10 to print the register name */

    mov al, 0x58 /*!< Move the ASCII value of 'X' to al */
    int 0x10 /*!< Call BIOS interrupt 0x10 to print 'X' */

    mov al, 0x3a /*!< Move the ASCII value of ':' to al */
    int 0x10 /*!< Call BIOS interrupt 0x10 to print ':' */

/*!
 * @brief Prints a register value in hexadecimal.
 * 
 * This routine prints a register value in hexadecimal format.
 */
no_reg:
    add bp, 2 /*!< Move to the next register on the stack */
    call print_hex /*!< Call print_hex to print the register value in hexadecimal */
    pop cx /*!< Restore cx from the stack */
    loop print_loop /*!< Loop back to print_loop if cx is not zero */
    ret /*!< Return from the function */

/*!
 * @brief Prints a newline.
 * 
 * This routine prints a newline (CR LF) for readability.
 */
print_nl:
    mov ax, 0xe0d /*!< Move the ASCII value of carriage return (CR) to ax */
    int 0x10 /*!< Call BIOS interrupt 0x10 to print CR */
    mov al, 0xa /*!< Move the ASCII value of line feed (LF) to al */
    int 0x10 /*!< Call BIOS interrupt 0x10 to print LF */
    ret /*!< Return from the function */

/*!
 * @brief Prints a word in hexadecimal format.
 * 
 * This routine prints the word pointed to by ss:bp in hexadecimal format.
 */
print_hex:
    mov cx, 4 /*!< Set cx to 4 (4 hexadecimal digits) */
    mov dx, [bp] /*!< Load the word at ss:bp into dx */

print_digit:
    rol dx, 4 /*!< Rotate dx left by 4 bits to isolate the next nibble */
    mov ah, 0xe /*!< Set ah to 0xe for BIOS teletype output function */
    mov al, dl /*!< Move the lower 4 bits of dx to al */
    and al, 0xf /*!< Mask al to isolate the nibble */

    add al, 0x30 /*!< Convert al to a digit (0-9) */
    cmp al, 0x39 /*!< Compare al with 0x39 to check if it is a digit */
    jbe good_digit /*!< Jump to good_digit if al is a digit */
    add al, 0x41 - 0x30 - 0xa /*!< Convert al to a letter (A-F) */
good_digit:
    int 0x10 /*!< Call BIOS interrupt 0x10 to print the digit */
    loop print_digit /*!< Loop back to print_digit if cx is not zero */
    ret /*!< Return from the function */

