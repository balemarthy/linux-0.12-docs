# Old Linux Kernel Source Study

This repository contains the source code for the old Linux kernel version 0.12. This is intended for educational purposes to understand and study the early versions of the Linux kernel.

## This is the documentation of the file bootsect.S

## File: `bootsect.S`

The `bootsect.S` file is the boot sector code for the Linux kernel. It is responsible for loading the kernel into memory and starting its execution. Below is a detailed explanation of the first 200 lines of this file.

### Detailed Explanation of `bootsect.S`

1. **BIOS Initialization**:
   - The BIOS loads the boot sector into memory at address `0x7c00` and sets the instruction pointer to `0x7c00`, making the CPU execute the boot sector code.

2. **Loading the Boot Sector**:
   - The boot sector code relocates itself from `0x7c00` to `0x90000` to free up the initial memory area for other uses.

3. **Relocating the Boot Sector**:
   - The code at `0x7c00` copies itself to `0x90000` using the `rep movsw` instruction to move 512 bytes.
   - After copying, it jumps to the new location at `0x90000` using the `jmpi` instruction.

4. **Setting Up Segments**:
   - The segment registers (`ds`, `es`, `ss`, `cs`) are set to `INITSEG` to manage different memory areas.
   - The stack pointer (`sp`) is set to a safe location.

5. **Creating Disk Parameter Tables**:
   - New disk parameter tables are created in RAM for multi-sector reads, setting the maximum sector count to 18.
   - This is done by copying 12 bytes of data to the appropriate memory locations.

6. **Resetting the Floppy Disk Controller**:
   - The floppy disk controller is reset using the `int 0x13` interrupt.

7. **Loading Setup Sectors**:
   - The setup sectors are loaded directly after the boot block using BIOS interrupts.

### Doxygen Comments for `bootsect.S`

The following Doxygen comments are added to elaborate on each line of the first 100 lines of `bootsect.S`:

```assembly
/*!
 * Move the boot sector to the new segment
 * 
 * This section of code sets up the data segment (DS) and extra segment (ES) to point to 
 * the original boot sector segment (BOOTSEG) and the new initialization segment (INITSEG), respectively.
 * The 'mov ax, #BOOTSEG' instruction loads the original boot sector segment address into the AX register.
 * The 'mov ds, ax' instruction then sets the data segment (DS) to this address.
 * Similarly, 'mov ax, #INITSEG' loads the new segment address into AX, and 'mov es, ax' sets the extra segment (ES) to this address.
 * The CX register is loaded with 256, representing the number of words (16-bit units) to be copied, equating to 512 bytes.
 * The source index (SI) and destination index (DI) registers are cleared (set to 0) to start copying from the beginning of the source and destination segments.
 */
mov ax,#BOOTSEG   /*!< Load the original boot sector segment into AX */
mov ds,ax         /*!< Set DS to the original boot sector segment */
mov ax,#INITSEG   /*!< Load the new boot sector segment into AX */
mov es,ax         /*!< Set ES to the new boot sector segment */
mov cx,#256       /*!< Set CX to 256, the number of words to copy (512 bytes) */
sub si,si         /*!< Set SI to 0, the source index */
sub di,di         /*!< Set DI to 0, the destination index */
/*!
 * Copy 512 bytes from BOOTSEG to INITSEG
 * 
 * The 'rep' instruction repeats the following 'movw' instruction CX times, 
 * effectively copying 256 words (or 512 bytes) from the source index (SI) in the data segment (DS) 
 * to the destination index (DI) in the extra segment (ES).
 */
rep               /*!< Repeat the following instruction CX times */
movw              /*!< Move word from DS:SI to ES:DI */
/*!
 * Jump to the new location at INITSEG
 * 
 * After copying the boot sector to the new segment, the 'jmpi go, INITSEG' instruction jumps to the 'go' label 
 * in the new segment, effectively transferring control to the new segment.
 */
jmpi go,INITSEG   /*!< Jump to the 'go' label in the new segment */

/*!
 * Set up data segments
 * 
 * The 'go' label marks the start of the code in the new segment. 
 * The 'mov ax, cs' instruction copies the code segment (CS) into the AX register, 
 * and 'mov dx, #0xfef4' loads an arbitrary value into the DX register. 
 * This value is used later to set up the stack pointer.
 */
go: mov ax,cs     /*!< Copy CS to AX */
mov dx,#0xfef4    /*!< Load an arbitrary value into DX */

/*!
 * Set up data segments
 * 
 * The data segment (DS) and extra segment (ES) are both set to the value in AX (the code segment).
 * The AX register is then pushed onto the stack for later use.
 */
mov ds,ax         /*!< Set DS to the value in AX */
mov es,ax         /*!< Set ES to the value in AX */
push ax           /*!< Push AX onto the stack */

/*!
 * Set up stack segment and pointer
 * 
 * The stack segment (SS) is set to the value in AX (the code segment), 
 * and the stack pointer (SP) is set to the value in DX (0xfef4). 
 * This sets up the stack in a known location.
 */
mov ss,ax         /*!< Set SS to the value in AX */
mov sp,dx         /*!< Set SP to the value in DX */

/*!
 * Create new disk parameter tables in RAM for multi-sector reads
 * 
 * This section of code sets up new disk parameter tables in RAM for multi-sector reads.
 * The 'push #0' and 'pop fs' instructions clear the FS segment register by pushing 0 onto the stack and then popping it into FS.
 * The 'mov bx, #0x78' instruction sets BX to the address of the parameter table.
 * The 'seg fs' and 'lgs si, (bx)' instructions load the global segment (GS) and source index (SI) with the value at the parameter table address.
 */
push #0           /*!< Push 0 onto the stack */
pop fs            /*!< Pop the stack value into FS */
mov bx,#0x78      /*!< Set BX to 0x78, the parameter table address */
seg fs
lgs si,(bx)       /*!< Load GS:SI with the value at FS:BX */

/*!
 * Copy parameter table to new location
 * 
 * The 'mov di, dx' instruction sets the destination index (DI) to the value in DX.
 * The 'mov cx, #6' instruction sets CX to 6, representing the number of words (12 bytes) to copy.
 * The 'cld' instruction clears the direction flag to ensure the copy operation increments the index registers.
 * The 'rep movw' instruction repeats the word move instruction CX times, copying 12 bytes from GS:SI to ES:DI.
 */
mov di,dx         /*!< Set DI to the value in DX */
mov cx,#6         /*!< Set CX to 6, the number of words to copy */
cld               /*!< Clear the direction flag */

rep               /*!< Repeat the following instruction CX times */
seg gs
movw              /*!< Move word from GS:SI to ES:DI */

/*!
 * Patch the sector count
 * 
 * The 'mov di, dx' instruction sets DI to the value in DX again.
 * The 'movb 4(di), *18' instruction patches the sector count at offset 4 in DI to 18.
 * The 'seg fs' and 'mov (bx), di' instructions store DI at the parameter table address.
 * The 'seg fs' and 'mov 2(bx), es' instructions store ES at the parameter table address+2.
 */
mov di,dx         /*!< Set DI to the value in DX */
movb 4(di),*18    /*!< Patch the sector count at offset 4 in DI */

seg fs
mov (bx),di       /*!< Store DI at FS:BX */
seg fs
mov 2(bx),es      /*!< Store ES at FS:BX+2 */

/*!
 * Restore segment registers
 * 
 * The 'pop ax' instruction pops the stack value into AX.
 * The 'mov fs, ax' and 'mov gs, ax' instructions set FS and GS to the value in AX.
 */
pop ax            /*!< Pop the stack value into AX */
mov fs,ax         /*!< Set FS to the value in AX */
mov gs,ax         /*!< Set GS to the value in AX */

/*!
 * Reset the floppy disk controller
 * 
 * The 'xor ah, ah' and 'xor dl, dl' instructions clear the AH and DL registers.
 * The 'int 0x13' instruction calls interrupt 0x13 to reset the floppy disk controller.
 */
xor ah,ah         /*!< Clear AH */
xor dl,dl         /*!< Clear DL */
int 0x13          /*!< Call interrupt 0x13 */

/*!
 * Load the setup sectors directly after the boot block
 * 
 * This section of code loads the setup sectors directly after the boot block using BIOS interrupts.
 * The 'xor dx, dx' instruction clears the DX register, setting the drive to 0 and the head to 0.
 * The 'mov cx, #0x0002' instruction sets CX to sector 2, track 0.
 * The 'mov bx, #0x0200' instruction sets BX to 512, representing the address in INITSEG.
 * The 'mov ax, #0x0200 + SETUPLEN' instruction sets AX to service 2 and the number of sectors to read.
 * The 'int 0x13' instruction calls interrupt 0x13 to read the sectors.
 * The 'jnc ok_load_setup' instruction jumps to 'ok_load_setup' if no carry (no error).
 */
load_setup:
xor dx, dx        /*!< Clear DX */
mov cx,#0x0002    /*!< Set CX to 2, sector 2, track 0 */
mov bx,#0x0200    /*!< Set BX to 512, the address in INITSEG */
mov ax,#0x0200+SETUPLEN /*!< Set AX to service 2, number of sectors */
int 0x13          /*!< Call interrupt 0x13 */
jnc ok_load_setup /*!< Jump if no carry (no error) */

/*!
 * Handle errors during sector loading
 * 
 * If an error occurs during sector loading, the error code is pushed onto the stack and printed using 'print_nl' and 'print_hex'.
 * The 'xor dl, dl' and 'xor ah, ah' instructions clear the DL and AH registers, and 'int 0x13' resets the floppy disk controller.
 * The 'j load_setup' instruction jumps back to 'load_setup' to retry the operation.
 */
push ax           /*!< Push AX onto the stack */
call print_nl     /*!< Call print_nl */
mov bp, sp        /*!< Set BP to SP */
call print_hex    /*!< Call print_hex */
pop ax            /*!< Pop the stack value into AX */

xor dl, dl        /*!< Clear DL */
xor ah, ah        /*!< Clear AH */
int 0x13          /*!< Call interrupt 0x13 */
j load_setup      /*!< Jump to load_setup */

ok_load_setup:

/*!
 * Get disk drive parameters, specifically number of sectors per track
 * 
 * The 'xor dl, dl' instruction clears the DL register.
 * The 'mov ah, #0x08' instruction sets AH to 8, the BIOS function to get drive parameters.
 * The 'int 0x13' instruction calls interrupt 0x13 to get the drive parameters.
 * The 'xor ch, ch' instruction clears the CH register.
 * The 'seg cs' and 'mov sectors, cx' instructions move the number of sectors per track from CX to the 'sectors' variable.
 * The 'mov ax, #INITSEG' and 'mov es, ax' instructions set ES to INITSEG.
 */
xor dl,dl         /*!< Clear DL */
mov ah,#0x08      /*!< Set AH to 8, get drive parameters */
int 0x13          /*!< Call interrupt 0x13 */
xor ch,ch         /*!< Clear CH */
seg cs
mov sectors,cx    /*!< Move CX to sectors */
mov ax,#INITSEG   /*!< Set AX to INITSEG */
mov es,ax         /*!< Set ES to AX */

/*!
 * Print a message
 * 
 * The 'mov ah, #0x03' instruction sets AH to 3, the BIOS function to read the cursor position.
 * The 'xor bh, bh' instruction clears BH.
 * The 'int 0x10' instruction calls interrupt 0x10 to read the cursor position.
 * The 'mov cx, #9' instruction sets CX to 9, the length of the message.
 * The 'mov bx, #0x0007' instruction sets BX to page 0, attribute 7 (normal).
 * The 'mov bp, #msg1' instruction sets BP to the address of the message.
 * The 'mov ax, #0x1301' instruction sets AX to the BIOS function to write a string and move the cursor.
 * The 'int 0x10' instruction calls interrupt 0x10 to print the message.
 */
mov ah,#0x03      /*!< Set AH to 3, read cursor position */
xor bh,bh         /*!< Clear BH */
int 0x10          /*!< Call interrupt 0x10 */

mov cx,#9         /*!< Set CX to 9 */
mov bx,#0x0007    /*!< Set BX to page 0, attribute 7 (normal) */
mov bp,#msg1      /*!< Set BP to msg1 */
mov ax,#0x1301    /*!< Set AX to write string, move cursor */
int 0x10          /*!< Call interrupt 0x10 */

/*!
 * Load the system (at 0x10000)
 * 
 * The 'mov ax, #SYSSEG' instruction sets AX to the system segment address.
 * The 'mov es, ax' instruction sets ES to this address.
 * The 'call read_it' instruction calls the 'read_it' function to load the system.
 * The 'call kill_motor' instruction calls the 'kill_motor' function to turn off the floppy drive motor.
 * The 'call print_nl' instruction calls the 'print_nl' function to print a newline.
 */
mov ax,#SYSSEG    /*!< Set AX to SYSSEG */
mov es,ax         /*!< Set ES to AX */
call read_it      /*!< Call read_it */
call kill_motor   /*!< Call kill_motor */
call print_nl     /*!< Call print_nl */

/*!
 * Check which root device to use
 * 
 * The 'seg cs' and 'mov ax, root_dev' instructions move the root device value into AX.
 * The 'or ax, ax' instruction checks if the device is defined.
 * The 'jne root_defined' instruction jumps to 'root_defined' if the device is defined.
 * The 'seg cs' and 'mov bx, sectors' instructions move the number of sectors per track into BX.
 * The 'mov ax, #0x0208' instruction sets AX to the device identifier for /dev/ps0 (1.2Mb).
 * The 'cmp bx, #15' instruction compares the number of sectors with 15, and 'je root_defined' jumps to 'root_defined' if equal.
 * The 'mov ax, #0x021c' instruction sets AX to the device identifier for /dev/PS0 (1.44Mb).
 * The 'cmp bx, #18' instruction compares the number of sectors with 18, and 'je root_defined' jumps to 'root_defined' if equal.
 * The 'undef_root' label and 'jmp undef_root' instruction create an infinite loop if the root device is undefined.
 * The 'root_defined' label and 'seg cs' and 'mov root_dev, ax' instructions set the root device value.
 */
seg cs
mov ax,root_dev   /*!< Move root_dev into AX */
or ax,ax          /*!< OR AX with AX */
jne root_defined  /*!< Jump if not equal to root_defined */
seg cs
mov bx,sectors    /*!< Move sectors into BX */
mov ax,#0x0208    /*!< Set AX to /dev/ps0 - 1.2Mb */
cmp bx,#15        /*!< Compare BX with 15 */
je root_defined   /*!< Jump if equal to root_defined */
mov ax,#0x021c    /*!< Set AX to /dev/PS0 - 1.44Mb */
cmp bx,#18        /*!< Compare BX with 18 */
je root_defined   /*!< Jump if equal to root_defined */
undef_root:
jmp undef_root    /*!< Jump to undef_root */
root_defined:
seg cs
mov root_dev,ax   /*!< Move AX to root_dev */

/*!
 * Jump to the setup routine loaded directly after the boot block
 * 
 * The 'jmpi 0, SETUPSEG' instruction jumps to offset 0 in the setup segment.
 * This transfers control to the setup routine loaded directly after the boot block.
 */
jmpi 0,SETUPSEG   /*!< Jump to 0 in SETUPSEG */


