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
