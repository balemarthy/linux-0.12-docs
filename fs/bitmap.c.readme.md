# README - Linux 0.12 `bitmap.c`

## Overview

The `bitmap.c` file in Linux 0.12 is responsible for managing **inode and block bitmaps**. It provides functions for:

- **Allocating and freeing disk blocks** using a bitmap representation.
- **Finding the first available free bit** in a bitmap.
- **Managing inodes**, which store file metadata.

This mechanism ensures **efficient allocation and deallocation of filesystem resources**.

---

## **Understanding Inline Assembly (`__asm__`) in Macros**

The `__asm__` keyword is used to embed **assembly instructions** within C code. These macros provide highly optimized **bitwise operations** for bitmap management.

### **Syntax Explanation:**

__asm__ ("assembly code"
         : output operands
         : input operands
         : clobbered registers);

- **`assembly code`** → The actual assembly instructions.
- **`output operands`** → Variables modified by the assembly.
- **`input operands`** → Variables passed to the assembly code.
- **`clobbered registers`** → Registers modified by assembly that the compiler must be aware of.

These macros are used for **direct manipulation of memory** to speed up bitmap operations.

---

## **Functional Breakdown (Line-by-Line)**

### **1. Clearing a Block (`clear_block`)**

#define clear_block(addr) \
__asm__("cld\n\t" \
    "rep\n\t" \
    "stosl" \
    :"=D" (addr) \
    :"a" (0), "c" (BLOCK_SIZE/4), "D" ((long) (addr)))


#### **Explanation:**
- **Purpose:** Clears a memory block by filling it with zeros.
- **How does it work?**
  - **`cld`** → Clears the direction flag to ensure forward memory copying.
  - **`rep stosl`** →
    - `rep` repeats `stosl` (store string long) until `ecx` reaches 0.
    - `stosl` writes `EAX` (0) to the memory pointed by `EDI`.
    - `BLOCK_SIZE/4` ensures **4-byte alignment**.
- **Why is this needed?**
  - Ensures **data integrity** before reusing a block.
  - Assembly-based implementation is **faster** than a standard C loop.

---

### **2. Setting a Bit (`set_bit`)**

#define set_bit(nr, addr) ({\
register int res __asm__("ax"); \
__asm__ __volatile__("btsl %2,%3\n\tsetb %%al": \
"=a" (res):"0" (0),"r" (nr),"m" (*(addr))); \
res;})

#### **Explanation:**
- **Purpose:** Sets a bit at position `nr` in a bitmap.
- **How does it work?**
  - **`btsl %2, %3`** → **Bit Test and Set**.
  - **`setb %%al`** → Stores the previous bit value into `AL`.
  - The macro returns the previous bit value (1 if already set, 0 otherwise).
- **Why is this needed?**
  - Ensures **thread-safe bit operations**.
  - Prevents race conditions when modifying shared bitmaps.

---

### **3. Clearing a Bit (`clear_bit`)**

#define clear_bit(nr, addr) ({\
register int res __asm__("ax"); \
__asm__ __volatile__("btrl %2,%3\n\tsetnb %%al": \
"=a" (res):"0" (0),"r" (nr),"m" (*(addr))); \
res;})

#### **Explanation:**
- **Purpose:** Clears a specific bit in a bitmap.
- **How does it work?**
  - **`btrl %2,%3`** → **Bit Test and Reset**.
  - **`setnb %%al`** → Stores the previous bit value.
- **Why is this needed?**
  - Ensures atomicity in modifying shared memory bitmaps.


