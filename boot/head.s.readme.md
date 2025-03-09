# head.s - 32-bit Startup Code for Linux Kernel

This file contains the startup code for initializing the Linux kernel in 32-bit protected mode. It sets up the Global Descriptor Table (GDT), Interrupt Descriptor Table (IDT), and prepares the system for paging. This code executes at address 0x00000000, which will later be used for the page directory.

## Purpose
- **Initialization**: Sets up 32-bit protected mode.
- **Memory Management**: Prepares GDT and IDT.
- **Paging**: Sets up paging to manage memory efficiently.
- **System Checks**: Includes checks for A20 line and math coprocessor.

## Main Components
1. **GDT and IDT Setup**: Essential for memory segmentation and interrupt handling.
2. **Paging Setup**: Enables virtual memory by setting up page tables.
3. **System Checks**: Verifies the A20 line and math coprocessor functionality.

## Detailed Explanation

### Code


/**
 * @file head.s
 * @brief 32-bit startup code for the Linux kernel.
 *
 * This file contains the startup code for initializing the Linux kernel in 32-bit protected mode.
 * It sets up the Global Descriptor Table (GDT), Interrupt Descriptor Table (IDT), and prepares
 * the system for paging. This code executes at address 0x00000000, which will later be used for
 * the page directory.
 *
 * @note Startup happens at absolute address 0x00000000, which is also where the page directory
 *       will exist. The startup code will be overwritten by the page directory.
 */

/* Global declarations */
.text
.globl idt, gdt, pg_dir, tmp_floppy_area, startup_32

/**
 * @brief Page directory.
 */
pg_dir:

/**
 * @brief Entry point for 32-bit startup code.
 */
startup_32:
    movl $0x10, %eax /**< Load data segment selector into eax. */
    mov %ax, %ds     /**< Load data segment selector into ds. */
    mov %ax, %es     /**< Load data segment selector into es. */
    mov %ax, %fs     /**< Load data segment selector into fs. */
    mov %ax, %gs     /**< Load data segment selector into gs. */
    lss stack_start, %esp /**< Load stack pointer. */
    call setup_idt   /**< Call function to set up IDT. */
    call setup_gdt   /**< Call function to set up GDT. */
    movl $0x10, %eax /**< Reload all segment registers after changing GDT. */
    mov %ax, %ds     /**< Reload data segment selector into ds. */
    mov %ax, %es     /**< Reload data segment selector into es. */
    mov %ax, %fs     /**< Reload data segment selector into fs. */
    mov %ax, %gs     /**< Reload data segment selector into gs. */
    lss stack_start, %esp /**< Reload stack pointer. */
    xorl %eax, %eax  /**< Clear eax register. */
1:  incl %eax        /**< Increment eax to check that A20 line is enabled. */
    movl %eax, 0x000000 /**< Loop forever if A20 line is not enabled. */
    cmpl %eax, 0x100000
    je 1b

/*
 * @note For 486 processors, set bit 16 to check for write-protect in supervisor mode.
 *       It is recommended to set the NE bit to use int 16 for math errors.
 */
    movl %cr0, %eax    /**< Check math coprocessor. */
    andl $0x80000011, %eax /**< Save PG, PE, and ET bits. */
    orl $2, %eax       /**< Set MP bit. */
    movl %eax, %cr0
    call check_x87     /**< Call function to check x87 coprocessor. */
    jmp after_page_tables /**< Jump to after_page_tables label. */

/**
 * @brief Check for x87 coprocessor presence.
 */
check_x87:
    fninit            /**< Initialize x87 coprocessor. */
    fstsw %ax         /**< Store x87 status word in ax. */
    cmpb $0, %al
    je 1f             /**< If no coprocessor, set bits. */
    movl %cr0, %eax
    xorl $6, %eax     /**< Reset MP bit and set EM bit. */
    movl %eax, %cr0
    ret
.align 4
1:  .byte 0xDB, 0xE4 /**< fsetpm for 287 coprocessor, ignored by 387. */
    ret

/**
 * @brief Set up the Interrupt Descriptor Table (IDT).
 *
 * This function sets up an IDT with 256 entries pointing to ignore_int interrupt gates.
 * It then loads the IDT. Everything that wants to install itself in the IDT table may do so
 * themselves. Interrupts are enabled elsewhere, when we can be relatively sure everything is okay.
 * This routine will be overwritten by the page tables.
 */
setup_idt:
    lea ignore_int, %edx /**< Load address of ignore_int into edx. */
    movl $0x00080000, %eax
    movw %dx, %ax    /**< Load selector 0x0008 (cs) into ax. */
    movw $0x8E00, %dx /**< Set interrupt gate, dpl=0, present. */
    lea idt, %edi    /**< Load address of IDT into edi. */
    mov $256, %ecx   /**< Load 256 entries count into ecx. */
rp_sidt:
    movl %eax, (%edi) /**< Copy eax to IDT entry. */
    movl %edx, 4(%edi) /**< Copy edx to IDT entry. */
    addl $8, %edi    /**< Move to next IDT entry. */
    dec %ecx         /**< Decrement counter. */
    jne rp_sidt      /**< Repeat for all entries. */
    lidt idt_descr   /**< Load IDT descriptor. */
    ret

/**
 * @brief Set up the Global Descriptor Table (GDT).
 *
 * This function sets up a new GDT and loads it. Only two entries are currently built,
 * the same ones that were built in init.s. The routine is very simple but is required
 * for the proper functioning of the kernel. This routine will be overwritten by the page tables.
 */
setup_gdt:
    lgdt gdt_descr   /**< Load GDT descriptor. */
    ret

/**
 * @brief Page tables for the kernel.
 *
 * The kernel page tables are placed right after the page directory,
 * using 4 of them to span 16 MB of physical memory. If the system has more than 16MB,
 * this needs to be expanded.
 */
.org 0x1000
pg0:

.org 0x2000
pg1:

.org 0x3000
pg2:

.org 0x4000
pg3:

/**
 * @brief Temporary floppy area.
 *
 * This area is used by the floppy driver when DMA cannot reach a buffer block.
 * It needs to be aligned so that it isn't on a 64KB border.
 */
.org 0x5000
tmp_floppy_area:
    .fill 1024, 1, 0

/**
 * @brief Parameters to main and the setup of paging.
 *
 * These lines push parameters to the main function and set up paging.
 */
after_page_tables:
    pushl $0          /**< Push parameter 0 to the stack. */
    pushl $0          /**< Push parameter 0 to the stack. */
    pushl $0          /**< Push parameter 0 to the stack. */
    pushl $L6         /**< Push return address for main. */
    pushl $main       /**< Push address of main function. */
    jmp setup_paging  /**< Jump to setup_paging. */
L6:
    jmp L6            /**< Main should never return here, but just in case, loop indefinitely. */

/**
 * @brief Default interrupt handler.
 *
 * This is the default interrupt handler which prints "Unknown interrupt" message.
 */
int_msg:
    .asciz "Unknown interrupt\n\r"
.align 4
ignore_int:
    pushl %eax        /**< Push eax to the stack. */
    pushl %ecx        /**< Push ecx to the stack. */
    pushl %edx        /**< Push edx to the stack. */
    push %ds          /**< Push ds to the stack. */
    push %es          /**< Push es to the stack. */
    push %fs          /**< Push fs to the stack. */
    movl $0x10, %eax  /**< Load data segment selector into eax. */
    mov %ax, %ds      /**< Load data segment selector into ds. */
    mov %ax, %es      /**< Load data segment selector into es. */
    mov %ax, %fs      /**< Load data segment selector into fs. */
    pushl $int_msg    /**< Push address of int_msg to the stack. */
    call printk       /**< Call printk function. */
    popl %eax         /**< Restore eax from the stack. */
    pop %fs           /**< Restore fs from the stack. */
    pop %es           /**< Restore es from the stack. */
    pop %ds           /**< Restore ds from the stack. */
    popl %edx         /**< Restore edx from the stack. */
    popl %ecx         /**< Restore ecx from the stack. */
    popl %eax         /**< Restore eax from the stack. */
    iret              /**< Return from interrupt. */

/**
 * @brief Set up paging.
 *
 * This routine sets up paging by setting the page bit in cr0. The page tables are
 * set up to identity-map the first 16MB of physical memory. The pager assumes that
 * no illegal addresses are produced.
 *
 * @note Although all physical memory should be identity mapped by this routine,
 * only the kernel page functions use the >1MB addresses directly. All "normal"
 * functions use just the lower 1MB, or the local data space, which will be mapped
 * to some other place. Memory management keeps track of that.
 */
.align 4
setup_paging:
    movl $1024*5, %ecx /**< Load 5 pages count into ecx. */
    xorl %eax, %eax   /**< Clear eax register. */
    xorl %edi, %edi   /**< Clear edi register, pointing to pg_dir. */
    cld; rep; stosl   /**< Clear 5 pages of memory. */
    movl $pg0+7, pg_dir /**< Set present bit/user r/w for pg0. */
    movl $pg1+7, pg_dir+4 /**< Set present bit/user r/w for pg1. */
    movl $pg2+7, pg_dir+8 /**< Set present bit/user r/w for pg2. */
    movl $pg3+7, pg_dir+12 /**< Set present bit/user r/w for pg3. */
    movl $pg3+4092, %edi
    movl $0xfff007, %eax /**< Set 16MB - 4096 + 7 (r/w user, present). */
    std
1:  stosl             /**< Fill pages backwards for efficiency. */
    subl $0x1000, %eax
    jge 1b
    cld
    xorl %eax, %eax   /**< Clear eax register. */
    movl %eax, %cr3   /**< Load page directory start into cr3. */
    movl %cr0, %eax
    orl $0x80000000, %eax /**< Set paging (PG) bit. */
    movl %eax, %cr0
    ret               /**< Flush prefetch queue. */

/**
 * @brief IDT descriptor.
 */
.align 4
idt_descr:
    .word 256*8-1    /**< IDT contains 256 entries. */
    .long idt
.align 4

/**
 * @brief GDT descriptor.
 */
.align 4
gdt_descr:
    .word 256*8-1    /**< GDT contains 256 entries. */
    .long gdt
.align 8

/**
 * @brief IDT table, uninitialized.
 */
idt: .fill 256, 8, 0

/**
 * @brief GDT table.
 */
gdt:
    .quad 0x0000000000000000 /**< NULL descriptor. */
    .quad 0x00c09a0000000fff /**< 16MB code segment. */
    .quad 0x00c0920000000fff /**< 16MB data segment. */
    .quad 0x0000000000000000 /**< Temporary, do not use. */
    .fill 252, 8, 0          /**< Space for LDTs and TSSs. */
