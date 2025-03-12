# README - Linux 0.12 `setup.S` Line-by-Line Explanation

## Overview

The `setup.S` file is responsible for retrieving system parameters from the **BIOS**, configuring memory, initializing the **interrupt controller (8259A)**, enabling **protected mode**, and setting up the **Global Descriptor Table (GDT) and Interrupt Descriptor Table (IDT)** before handing control over to the kernel.

## **Key Functionalities**
1. **Retrieves System Information:**
   - Detects **extended memory size** from the BIOS.
   - Determines **video mode** and screen resolution.
   - Fetches **hard disk parameters**.
2. **Moves the System Module** to the correct **memory location**.
3. **Enables Address Line A20** to allow access to memory above **1MB**.
4. **Initializes the 8259A Programmable Interrupt Controller**.
5. **Enables Protected Mode** by setting up **segment descriptors** and the **CPU mode switch**.

---

## **Code Breakdown - Function by Function**

### **1. Initialize Memory Segments**
```assembly
INITSEG  equ 0x9000   ; Bootloader segment
SYSSEG   equ 0x1000   ; System segment
SETUPSEG equ 0x9020   ; Setup segment

start:
    mov ax, INITSEG     ; Set data segment to INITSEG
    mov ds, ax
```
- Defines the memory segments and **sets up the data segment**.

### **2. Retrieve System Information from BIOS**
#### **Get Extended Memory Size**
```assembly
    mov ah, 0x88        ; BIOS Interrupt 0x15, function AH=0x88
    int 0x15           ; Get the size of extended memory (KB)
    mov [2], ax        ; Store extended memory size at memory offset 2
```
- Calls **BIOS INT 0x15** to obtain **available memory beyond 1MB**.

#### **Get Video Mode and Display Parameters**
```assembly
    mov ah, 0x12        ; BIOS Interrupt 0x10, function AH=0x12
    mov bl, 0x10        ; Query display type (EGA/VGA)
    int 0x10
    mov [8], ax        ; Store video information
    mov [10], bx
    mov [12], cx
```
- Calls **BIOS INT 0x10 (AH=0x12, BL=0x10)** to check if **VGA or EGA** is present.
- Stores video parameters at offsets **8, 10, and 12**.

#### **Detect Current Screen Row and Column**
```assembly
    mov ax, 0x5019    ; Default VGA mode value
    cmp bl, 0x10      ; If VGA is detected, no special check needed
    je novga
    call chsvga       ; Otherwise, call SVGA detection routine
novga:
    mov [14], ax      ; Store detected screen row and column values
```
- Calls **`chsvga`** to **detect non-standard SVGA** configurations.

#### **Retrieve Cursor Position**
```assembly
    mov ah, 0x03      ; BIOS Interrupt 0x10, function AH=0x03
    xor bh, bh        ; Set display page to 0
    int 0x10          ; Get cursor position
    mov [0], dx       ; Store cursor position at offset 0x90000
```
- Calls **BIOS INT 0x10 (AH=0x03)** to obtain **current cursor position**.

#### **Retrieve Hard Disk Parameters**
```assembly
    mov ax, 0x0000    ; Reset DS to 0 for BIOS data access
    mov ds, ax
    lds si, [4*0x41]  ; Load primary hard disk parameters
    mov ax, INITSEG
    mov es, ax
    mov di, 0x0080    ; Destination in memory
    mov cx, 0x10      ; Copy 16 bytes
    rep movsb         ; Copy data
```
- Retrieves **primary hard disk parameters** using **BIOS data segment**.

---

### **3. Move the System to the Correct Memory Location**
```assembly
    cli              ; Disable interrupts
    cld              ; Clear direction flag (moves forward)
    mov ax, 0x0000

do_move:
    mov es, ax       ; Set destination segment
    add ax, 0x1000
    cmp ax, 0x9000   ; Stop moving at segment 0x9000
    jz end_move
    mov ds, ax       ; Set source segment
    mov cx, 0x8000   ; Move 32KB at a time
    rep movsw        ; Copy data
    jmp do_move
```
- Moves the system module from **0x10000** to **0x00000** in **64KB chunks**.

---

### **4. Enable Address Line A20**
```assembly
    in al, 0x92       ; Read keyboard controller status
    or al, 0x02       ; Set bit 1 to enable A20
    out 0x92, al      ; Write back to enable A20 line
```
- Enables the **A20 address line** to allow memory access beyond **1MB**.

---

### **5. Initialize Interrupt Controller (8259A)**
```assembly
    mov al, 0x11     ; Start initialization sequence
    out 0x20, al     ; Send to master PIC
    out 0xA0, al     ; Send to slave PIC
```
- Initializes the **Programmable Interrupt Controller (PIC)**.

```assembly
    mov al, 0x20      ; Set master PIC base interrupt at 0x20
    out 0x21, al
    mov al, 0x28      ; Set slave PIC base interrupt at 0x28
    out 0xA1, al
```
- Remaps interrupts **to avoid conflicts** with CPU exception vectors.

---

### **6. Enter Protected Mode**
```assembly
    mov eax, cr0      ; Load CR0 register
    or eax, 0x01      ; Set protected mode bit (PE)
    mov cr0, eax      ; Write back to CR0
    jmp 0x08:0x00     ; Far jump to protected mode
```
- Enables **Protected Mode** by setting **bit 0 in CR0**.
- Performs a **far jump** to switch to **32-bit mode**.

---

## **Conclusion**
This document provides a **detailed breakdown** of the `setup.S` file, explaining how:
1. **System parameters are retrieved from BIOS**.
2. **The system is moved to the correct memory address**.
3. **Interrupt controllers and memory protection are initialized**.
4. **Protected mode is enabled**.

This code is essential for bootstrapping **Linux 0.12**, setting up the **hardware environment** before the kernel takes control.

---

This README file is structured to make it **GitHub-friendly**, with **function-by-function** and **line-by-line** explanations. Let me know if you need further refinements! 🚀

