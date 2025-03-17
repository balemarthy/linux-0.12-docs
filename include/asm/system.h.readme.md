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





