# Linux 0.12 - `setup.S` Line-by-Line Explanation

This document provides a **detailed explanation** of the `setup.S` file from **Linux 0.12**. This assembly file is responsible for retrieving system data from the BIOS and setting up memory for the operating system.

## Overview

### **Purpose of `setup.S`**
- Retrieves system parameters like **memory size, video mode, and disk information**.
- Stores retrieved information at **0x90000-0x901FF**.
- Moves the system to **protected mode** and sets up **interrupts**.

---

## Line-by-Line Explanation

### **Initialization and Segment Setup**

    mov ax, #INITSEG  ; Load INITSEG (Boot segment address)
    mov ds, ax        ; Set DS (Data Segment) to INITSEG

- The bootloader has already loaded `setup.S`, but we ensure the **data segment** is correctly set.


    mov ah, #0x88     ; BIOS call to get memory size
    int 0x15         ; Call interrupt 0x15 (returns memory size in KB)
    mov [2], ax      ; Store memory size at offset 2 in segment INITSEG

- Calls the **BIOS function (INT 0x15, AH=0x88)** to get available **extended memory** in KB.

---

### **Detecting Display Type and Configuration**

    mov ah, #0x12     ; BIOS function to get display information
    mov bl, #0x10     ; Set mode to VGA/EGA detection
    int 0x10          ; Call video BIOS
    mov [8], ax       ; Store EGA/VGA info at offset 8
    mov [10], bx      ; Store additional parameters
    mov [12], cx      ; Store more video data

- Calls BIOS **interrupt 0x10** to **detect VGA/EGA video modes**.
- Stores results at memory locations **8, 10, and 12**.


    mov ax, #0x5019   ; Default VGA identifier
    cmp bl, #0x10     ; If VGA/EGA detected, use default ID
    je novga          ; Jump if VGA detected
    call chsvga       ; Call routine to detect specific SVGA type
novga:
    mov [14], ax      ; Store detected VGA mode

- Checks if a **VGA-compatible card** is present.
- Calls `chsvga` function to detect **SVGA configurations**.

---

### **Getting Cursor Position**

    mov ah, #0x03     ; BIOS call to get cursor position
    xor bh, bh       ; Select page 0
    int 0x10         ; Call video BIOS
    mov [0], dx      ; Store cursor position at offset 0x90000

- Retrieves and stores **cursor position**, which will later be used for video initialization.

---

### **Getting Video Card Data**

    mov ah, #0x0F    ; Get video mode
    int 0x10        ; Call video BIOS
    mov [4], bx     ; Store video page in memory
    mov [6], ax     ; Store video mode and width

- Calls BIOS **interrupt 0x10 (AH=0x0F)** to get **current video mode**.
- Stores retrieved **mode and parameters**.

---

### **Retrieving Hard Disk Information**

    mov ax, #0x0000   ; Set DS=0 for BIOS data
    mov ds, ax
    lds si, [4*0x41]  ; Load address of primary hard disk table
    mov ax, #INITSEG  ; Destination segment
    mov es, ax
    mov di, #0x0080   ; Destination address for HD0 data
    mov cx, #0x10     ; 16 bytes to copy
    rep movsb         ; Copy HD0 data

- Copies **BIOS disk data** into **0x90080**.


    mov ax, #0x01500  ; Detect secondary hard disk (AH=0x15)
    mov dl, #0x81     ; Select second hard disk
    int 0x13         ; Call disk BIOS
    jc no_disk1      ; If carry flag is set, no second disk found
    cmp ah, #3       ; If AH=3, disk exists
    je is_disk1

- Uses BIOS **interrupt 0x13 (AH=0x15)** to check for a **secondary hard disk**.
- If disk **does not exist**, it clears the entry.

---

### **Moving the System to Its Correct Memory Location**

    cli             ; Disable interrupts (critical section)
    mov ax, #0x0000 ; Source segment (start from address 0)
    cld            ; Clear direction flag (move forward)
do_move:
    mov es, ax     ; Set destination segment
    add ax, #0x1000 ; Move in 64KB blocks
    cmp ax, #0x9000 ; Stop at 0x9000
    jz end_move
    mov ds, ax     ; Set source segment
    mov cx, #0x8000 ; Move 32KB per iteration
    rep movsw      ; Move data from DS to ES
    jmp do_move

- Moves the system **from low memory (0x0000-0x9000)** to its correct **higher memory location**.

---

### **Loading IDT and GDT (Interrupt & Descriptor Tables)**

end_move:
    mov ax, #SETUPSEG  ; Load setup segment
    mov ds, ax
    lidt idt_48       ; Load IDT with a 0,0 entry
    lgdt gdt_48       ; Load GDT for protected mode

- Loads **Interrupt Descriptor Table (IDT)** and **Global Descriptor Table (GDT)**.
- This prepares for switching to **protected mode**.

---

### **Enabling A20 (Address Line 20)**

    call empty_8042    ; Clear keyboard buffer
    mov al, #0xD1      ; Write command
    out #0x64, al     ; Send command to keyboard controller
    call empty_8042
    mov al, #0xDF      ; Enable A20
    out #0x60, al
    call empty_8042

- Enables **A20**, allowing memory access **beyond 1MB**.

---

### **Reprogramming the 8259 PIC (Programmable Interrupt Controller)**

    mov al, #0x11     ; Initialize sequence
    out #0x20, al    ; Send to master PIC
    out #0xA0, al    ; Send to slave PIC
    mov al, #0x20     ; Set master IRQ start (0x20)
    out #0x21, al
    mov al, #0x28     ; Set slave IRQ start (0x28)
    out #0xA1, al

- Reprograms the **interrupt controller** to avoid conflicts with CPU hardware interrupts.

---

### **Enabling Protected Mode**

    mov ax, #0x0001  ; Enable protected mode bit (PE=1)
    lmsw ax         ; Load Machine Status Word (enables protected mode)
    jmpi 0, 8       ; Jump to protected mode execution

- Sets **protected mode** by enabling the **PE bit in CR0**.
- Performs a **far jump (jmpi)** to **code segment 8**, entering 32-bit mode.

---

## **Conclusion**
This document provides a **detailed breakdown** of `setup.S`, explaining every instruction. The file:
- Retrieves **system information**.
- Moves the **system to its correct memory location**.
- Sets up **interrupts and protected mode**.

This is a foundational part of **Linux boot process**. 🚀

