# Inline Assembly Macros in Linux 0.12 

This document explains the inline **assembly macros** used in Kernel. These macros perform **low-level CPU operations**, such as **task switching, privilege level changes, and interrupt handling**.

---

## **1. Switching to User Mode (`move_to_user_mode()`)**

#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x17\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl $1f\n\t" \
	"iret\n" \
	"1:\tmovl $0x17,%%eax\n\t" \
	"mov %%ax,%%ds\n\t" \
	"mov %%ax,%%es\n\t" \
	"mov %%ax,%%fs\n\t" \
	"mov %%ax,%%gs" \
	:::"ax")


### **Explanation**
- **Purpose:** Switches execution from **kernel mode (ring 0)** to **user mode (ring 3)**.
- **Step-by-step breakdown:**
  1. **Save the stack pointer (ESP)**:

     movl %esp, %eax

     Stores the current stack pointer (`ESP`) in `EAX` for later use.
  2. **Push required values onto the stack** to prepare for `iret`:

     pushl $0x17      ; User mode data segment
     pushl %eax       ; Stack pointer
     pushfl           ; Push CPU flags
     pushl $0x0f      ; User mode code segment
     pushl $1f        ; Return address

  3. **Use `iret` (Interrupt Return) to switch privilege levels**:

     iret

     This pops the required values from the stack to:
     - Change the CPU privilege level to **user mode**.
     - Restore the **execution context**.
  4. **Update segment registers after returning to user mode**:

     movl $0x17, %eax
     mov %ax, %ds
     mov %ax, %es
     mov %ax, %fs
     mov %ax, %gs

     This sets the **data segment registers** (`ds`, `es`, `fs`, `gs`) to the **user mode segment** (`0x17`).

### **Why is this needed?**
- **Ensures safe transition from kernel to user mode**.
- The `iret` instruction is a trick to **safely drop privileges** and continue execution **in user mode**.

---

## **2. Enable Interrupts (`sti()`)**

#define sti() __asm__ ("sti"::)


### **Explanation**
- **Purpose:** Enables **hardware interrupts**.
- **How does it work?**
  - The **`sti`** (Set Interrupt Flag) instruction **allows** the CPU to process **pending and future interrupts**.
- **When is it used?**
  - After completing **critical operations**, when the system needs to **resume handling interrupts**.

---

## **3. Disable Interrupts (`cli()`)**

#define cli() __asm__ ("cli"::)


### **Explanation**
- **Purpose:** Disables **hardware interrupts**.
- **How does it work?**
  - The **`cli`** (Clear Interrupt Flag) instruction **prevents** the processor from responding to **external interrupts**.
- **When is it used?**
  - Before entering **critical sections** to **prevent race conditions and ensure atomic operations**.

---

## **4. No Operation (`nop()`)**

#define nop() __asm__ ("nop"::)


### **Explanation**
- **Purpose:** Executes a **no-operation** instruction.
- **How does it work?**
  - `nop` tells the CPU to **do nothing for one cycle**.
- **Use case:**
  - Used for **timing adjustments, instruction pipeline optimizations, and debugging**.

---

## **5. Interrupt Return (`iret()`)**

#define iret() __asm__ ("iret"::)


### **Explanation**
- **Purpose:** Returns from an **interrupt handler**, restoring:
  - **Instruction Pointer (EIP)**
  - **Code Segment (CS)**
  - **Flags (EFLAGS)**
  - **Stack Pointer (ESP)**
  - **Stack Segment (SS)**
- **How does it work?**
  - `iret` restores the CPU state to **continue execution from where the interrupt occurred**.
- **When is it used?**
  - At the **end of interrupt service routines (ISRs)**.

---

## **Summary of These Macros**
| **Macro** | **Purpose** | **Effect** |
|--------|-------------|--------|
| `move_to_user_mode()` | Switches CPU from **kernel mode** to **user mode** | Uses `iret` to change privilege level |
| `sti()` | Enables **interrupts** | Sets the **interrupt flag (IF=1)** |
| `cli()` | Disables **interrupts** | Clears the **interrupt flag (IF=0)** |
| `nop()` | Does **nothing** for one CPU cycle | Used for **timing, debugging, and optimizations** |
| `iret()` | Returns from an **interrupt** | Restores **CPU state and execution flow** |

---

## **Why Are These Macros Important?**
- **They provide fundamental control over CPU execution** in the Linux kernel.
- **Interrupt control (`sti()` & `cli()`)** is crucial for **multi-tasking and synchronization**.
- **`move_to_user_mode()` enables safe privilege changes** between kernel and user mode.
- **These macros ensure the operating system runs efficiently** without race conditions.

  ---
# **Gate Setup Macros in Linux 0.12**

## **Overview**
These macros define how **interrupts, traps, and system calls** are set up in the **Interrupt Descriptor Table (IDT)**.
They configure different types of **gates** used by the Linux kernel to handle CPU-level events.

---

## **6. `_set_gate()` - The Core Macro**

#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	  "o" (*((char *) (gate_addr))), \
	  "o" (*(4+(char *) (gate_addr))), \
	  "d" ((char *) (addr)),"a" (0x00080000))


### **Explanation**
- **Purpose:** Sets an entry in the **IDT (Interrupt Descriptor Table)**.
- **What does it do?**
  - Defines how an **interrupt or system call** is handled.
  - Configures an **interrupt, trap, or system gate** with specific privilege levels.

#### **Step-by-Step Breakdown**
**Move function address (`addr`) into register `DX`**:
   
   movw %%dx,%%ax
   
   - Copies the **lower 16 bits** of the handler address into `AX`.

**Move gate descriptor type and privilege level into `DX`**:
   
   movw %0,%%dx
   
   - The value **`0x8000 + (dpl << 13) + (type << 8)`** is calculated:
     - **`0x8000`** → Present bit (`P = 1`).
     - **`dpl << 13`** → Descriptor Privilege Level (DPL).
     - **`type << 8`** → Gate type (interrupt/trap).

**Store the descriptor into the IDT entry**:
   
   movl %%eax,%1
   movl %%edx,%2
   
   - Stores `EAX` in the **first half** of the IDT entry.
   - Stores `EDX` in the **second half**.

#### **Why is this needed?**
- The **IDT** is essential for handling **CPU interrupts**.
- This macro ensures correct setup for:
  - **Interrupt handlers (set_intr_gate)**.
  - **Trap handlers (set_trap_gate)**.
  - **System call handlers (set_system_gate)**.

---

## **7. `set_intr_gate()` - Setting Up Interrupt Handlers**

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)


### **Explanation**
- **Purpose:** Configures an **interrupt gate** in the **IDT**.
- **How does it work?**
  - Calls `_set_gate()` with:
    - **Type = 14** → Indicates an **interrupt gate**.
    - **DPL = 0** → **Kernel mode** only.
  - Stores the **handler function (`addr`)** at entry `n` in `idt`.

#### **Why is this needed?**
- **Interrupt handlers** need **interrupt gates** to:
  - **Automatically disable interrupts** during execution.
  - Ensure proper **CPU state preservation**.

---

## **8. `set_trap_gate()` - Setting Up Trap Handlers**

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)


### **Explanation**
- **Purpose:** Configures a **trap gate**.
- **How does it work?**
  - Calls `_set_gate()` with:
    - **Type = 15** → Indicates a **trap gate**.
    - **DPL = 0** → **Kernel mode only**.
  - The handler function (`addr`) is stored at entry `n` in `idt`.

#### **Why is this needed?**
- **Trap handlers** are used for **faults and exceptions** (e.g., page faults).
- Unlike interrupts, **trap gates do NOT disable interrupts** automatically.

---

## **9. `set_system_gate()` - Defining System Calls**

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)


### **Explanation**
- **Purpose:** Configures a **system call handler**.
- **How does it work?**
  - Calls `_set_gate()` with:
    - **Type = 15** → Trap gate (same as `set_trap_gate`).
    - **DPL = 3** → **User mode access** (**ring 3**).
  - The handler function (`addr`) is stored at entry `n` in `idt`.

#### **Why is this needed?**
- Allows **user-space programs** to make system calls.
- **DPL = 3** ensures that **user mode** code can trigger it.
- Unlike `set_intr_gate`, system calls do **not disable interrupts**.

---

## **Summary of Gate Macros**
| **Macro** | **Type** | **Privilege Level (DPL)** | **Use Case** |
|-----------|----------|----------------|------------|
| `_set_gate()` | Core function | Configurable | Sets up IDT entries |
| `set_intr_gate()` | Interrupt Gate | 0 (Kernel) | CPU interrupts |
| `set_trap_gate()` | Trap Gate | 0 (Kernel) | CPU exceptions, faults |
| `set_system_gate()` | Trap Gate | 3 (User) | System calls |

---

## **Why Are These Important?**
- **Interrupt and trap gates** control how the CPU handles:
  - **Interrupts (e.g., hardware events)**.
  - **Traps (e.g., software faults, system calls)**.
- The **correct privilege level (DPL)** ensures:
  - Kernel-level interrupts can only be executed by the **kernel**.
  - User-mode system calls are **permitted safely**.
- These macros allow **fine-grained control** over **Linux's interrupt handling**.

---
# Segment Descriptor Macros in Linux 0.12

## **Overview**
These macros define how **segment descriptors** are set up in the **Global Descriptor Table (GDT)** and **Local Descriptor Table (LDT)**.
Segment descriptors are used to define **code, data, and task state segments** in protected mode.

---

## **10. `_set_seg_desc()` - The Core Macro**

#define _set_seg_desc(gate_addr,type,dpl,base,limit) {
    *(gate_addr) = ((base) & 0xff000000) | \
        (((base) & 0x00ff0000)>>16) | \
        ((limit) & 0xf0000) | \
        ((dpl)<<13) | \
        (0x00408000) | \
        ((type)<<8); \
    *((gate_addr)+1) = (((base) & 0x0000ffff)<<16) | \
        ((limit) & 0x0ffff); }


### **Explanation**
- **Purpose:** Sets a **segment descriptor** in the **GDT/LDT**.
- **What does it do?**
  - Configures **code, data, or TSS segment descriptors**.
  - Specifies **segment base address, limit, and type**.

#### **Step-by-Step Breakdown**
 **First 32-bit word of the descriptor (`*(gate_addr)`)**:
   - **Extracts the base address**:
     - **High byte** (`base & 0xFF000000`) is stored.
     - **Middle byte** (`(base & 0x00FF0000) >> 16`) is extracted.
   - **Sets the segment limit** (`limit & 0xF0000`).
   - **Configures privilege level (DPL)** → `(dpl << 13)`.
   - **Descriptor flags:**
     - `0x00408000` enables present bit, granularity, and 32-bit operation.
     - `(type << 8)` sets the segment type (code, data, TSS).

 **Second 32-bit word of the descriptor (`*((gate_addr)+1)`)**:
   - Stores the **low 16-bits of base address** (`(base & 0x0000FFFF) << 16`).
   - Stores the **low 16-bits of the segment limit** (`limit & 0x0FFFF`).

#### **Why is this needed?**
- Defines memory segments used by the **kernel and processes**.
- Sets up the **protected mode memory model**.
- Used to create **code, data, and TSS descriptors**.

---

## **11. `_set_tssldt_desc()` - Setting Up TSS & LDT Descriptors**

#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
    "movw %%ax,%2\n\t" \
    "rorl $16,%%eax\n\t" \
    "movb %%al,%3\n\t" \
    "movb $" type ",%4\n\t" \
    "movb $0x00,%5\n\t" \
    "movb %%ah,%6\n\t" \
    "rorl $16,%%eax" \
    ::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
     "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
    )


### **Explanation**
- **Purpose:** Creates **Task State Segment (TSS) and Local Descriptor Table (LDT) entries**.
- **How does it work?**
  - Configures **104-byte TSS descriptor**.
  - Stores **TSS/LDT base address and limit** in the GDT.

#### **Step-by-Step Breakdown**
**Set the segment limit to 104 bytes**:

   movw $104, %1

   - TSS segments are **always 104 bytes** in size.

**Store the base address in two parts**:

   movw %%ax, %2
   rorl $16, %%eax
   movb %%al, %3

   - `movw %%ax, %2` → Stores **low 16 bits** of the base.
   - `rorl $16, %%eax` → Rotates right to extract next byte.
   - `movb %%al, %3` → Stores **middle byte**.

**Store the descriptor type**:

   movb $" type ", %4

   - Sets the type (`0x89` for TSS, `0x82` for LDT).

**Store high byte and reset `EAX`**:

   movb $0x00, %5
   movb %%ah, %6
   rorl $16, %%eax

   - **Finalizes the descriptor entry.**

#### **Why is this needed?**
- **TSS descriptors** are required for **task switching**.
- **LDT descriptors** define per-process memory segments.

---

## **12. `set_tss_desc()` - Setting Up Task State Segments**

#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x89")


### **Explanation**
- **Purpose:** Sets up a **Task State Segment (TSS) descriptor**.
- **How does it work?**
  - Calls `_set_tssldt_desc()` with:
    - **Type = `0x89`** → Indicates **TSS descriptor**.
    - **Base = `addr`** → Stores TSS base address.
  - Stores it in the **Global Descriptor Table (GDT)**.

#### **Why is this needed?**
- **TSS (Task State Segment)** is used for **hardware-based task switching**.
- Stores CPU **register state, stack pointers, and I/O permissions**.

---

## **13. `set_ldt_desc()` - Setting Up Local Descriptor Tables**

#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x82")


### **Explanation**
- **Purpose:** Sets up a **Local Descriptor Table (LDT) descriptor**.
- **How does it work?**
  - Calls `_set_tssldt_desc()` with:
    - **Type = `0x82`** → Indicates **LDT descriptor**.
    - **Base = `addr`** → Stores LDT base address.
  - Stores it in the **Global Descriptor Table (GDT)**.

#### **Why is this needed?**
- **LDT (Local Descriptor Table)** defines memory segments for **user-space processes**.
- Each process has its **own LDT**, separate from the **GDT**.

---

## **Summary of Segment Descriptor Macros**
| **Macro** | **Type** | **Description** |
|-----------|----------|----------------|
| `_set_seg_desc()` | Code/Data Segments | Defines memory segments in GDT/LDT |
| `_set_tssldt_desc()` | TSS/LDT Segments | Defines TSS/LDT in the GDT |
| `set_tss_desc()` | Task State Segment | Configures CPU task switching |
| `set_ldt_desc()` | Local Descriptor Table | Defines process-specific memory segments |

---

## **Why Are These Important?**
- **Memory segmentation** is crucial in **protected mode**.
- **TSS and LDTs** help manage **multitasking and memory protection**.
- These macros **define CPU memory layout** for **Linux 0.12**.










