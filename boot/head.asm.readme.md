# x86 Assembly System Initialization

This document provides a **line-by-line** explanation of the x86 assembly system initialization file. The assembly file is responsible for setting up essential system components, including the **IDT (Interrupt Descriptor Table), GDT (Global Descriptor Table), and Paging** before the operating system takes control.

## Overview

The file performs the following key operations:
- Initializes **protected mode**
- Sets up the **Interrupt Descriptor Table (IDT)**
- Configures the **Global Descriptor Table (GDT)**
- Ensures the **A20 line** is enabled
- Checks for an **x87 Floating Point Unit (FPU)**
- Sets up **paging** to enable virtual memory

---

## Line-by-Line Explanation

### Startup Routine
```assembly
startup_32:
    mov eax, 0x10    ; Load segment selector (0-8MB range) into eax
    mov ds, eax      ; Set data segment to 0x10
    mov es, eax      ; Set extra segment
    mov fs, eax      ; Set FS register
    mov gs, eax      ; Set GS register

    lss esp, [stack_start]  ; Load stack pointer from stack_start address
    call setup_idt  ; Initialize Interrupt Descriptor Table
    call setup_gdt  ; Initialize Global Descriptor Table

    mov eax, 0x10   ; Reload segment registers after setting up GDT/IDT
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    lss esp, [stack_start]  ; Reset stack pointer
```

---

### A20 Line Check
```assembly
    xor eax, eax       ; Clear eax register

test_A20:
    inc eax           ; Increment eax counter
    mov [0x0], eax    ; Write test value to low memory
    cmp eax, [0x100000] ; Compare with high memory address
    je test_A20       ; If equal, A20 is disabled, retry until enabled
```

---

### x87 Floating-Point Unit (FPU) Check
```assembly
    mov eax, cr0      ; Load control register CR0
    and eax, 0x80000011 ; Mask unnecessary bits
    or eax, 0x2       ; Set EM (Emulation bit) for FPU check
    mov cr0, eax      ; Write back to CR0
    call check_x87    ; Call FPU detection function
```
```assembly
check_x87:
    fninit           ; Initialize FPU
    fstsw ax        ; Store FPU status word in AX
    cmp al, 0       ; Check if FPU is present
    je .exist       ; If FPU exists, jump to .exist

    mov eax, cr0    ; Load CR0 register
    xor eax, 0x6    ; Disable FPU emulation
    mov cr0, eax    ; Write back to CR0
    ret

.exist:
    fsetpm          ; Set FPU protected mode
    ret
```

---

### Interrupt Descriptor Table (IDT) Setup
```assembly
setup_idt:
    mov edx, ignore_int ; Set default interrupt handler
    mov eax, 0x00080000 ; Set segment selector for IDT entries
    mov ax, dx
    mov dx, 0x8E00     ; Set interrupt gate attributes
    mov edi, idt       ; Load IDT address
    mov ecx, 256       ; Prepare for 256 IDT entries
```
```assembly
rp_sidt:
    mov [edi], eax  ; Store segment selector and attributes
    mov [edi+4], edx
    add edi, 0x8    ; Move to next IDT entry
    dec ecx         ; Decrement counter
    jne rp_sidt     ; Repeat until all 256 entries are set

    lidt [idt_descr] ; Load IDT into CPU
    ret
```

---

### Global Descriptor Table (GDT) Setup
```assembly
setup_gdt:
    lgdt [gdt_descr] ; Load GDT descriptor into CPU
    ret
```

---

### Paging Setup
```assembly
setup_paging:
    mov ecx, 1024*5 ; Number of pages to clear
    xor eax, eax     ; Clear eax register
    xor edi, edi     ; Clear edi (destination index)
    cld              ; Clear direction flag for rep stosd
    rep stosd        ; Clear page tables
```
```assembly
    mov dword [pg_dir],    pg0+7   ; Map first 4MB page
    mov dword [pg_dir+4],  pg1+7   ; Map second 4MB page
    mov dword [pg_dir+8],  pg2+7   ; Map third 4MB page
    mov dword [pg_dir+12], pg3+7   ; Map fourth 4MB page
```
```assembly
    mov cr3, eax      ; Load page directory base address
    mov eax, cr0
    or eax, 0x80000000 ; Enable paging by setting PG bit in CR0
    mov cr0, eax
    ret
```

---

## Descriptor Tables

### IDT Descriptor Table
```assembly
idt_descr:
    dw 256*8-1  ; IDT size (256 entries * 8 bytes each)
    dd idt      ; IDT base address
```

### GDT Descriptor Table
```assembly
gdt_descr:
    dw 256*8-1  ; GDT size (256 entries * 8 bytes each)
    dd gdt      ; GDT base address
```

---

## Conclusion

This document provides a **line-by-line breakdown** of the x86 assembly system initialization file. Each instruction is explained inline with comments to facilitate a clear understanding of how **protected mode, IDT, GDT, A20 line, FPU, and paging** are set up before an operating system takes control.

This should serve as a **detailed reference for system-level programming in x86 assembly**.

