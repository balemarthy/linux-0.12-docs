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

---

### **4. Finding the First Zero Bit (`find_first_zero`)**

#define find_first_zero(addr) ({ \
int __res; \
__asm__("cld\n" \
	"1:\tlodsl\n\t" \
	"notl %%eax\n\t" \
	"bsfl %%eax,%%edx\n\t" \
	"je 2f\n\t" \
	"addl %%edx,%%ecx\n\t" \
	"jmp 3f\n" \
	"2:\taddl $32,%%ecx\n\t" \
	"cmpl $8192,%%ecx\n\t" \
	"jl 1b\n" \
	"3:" \
	:"=c" (__res):"c" (0),"S" (addr):"ax","dx"); \
__res;})


#### **Explanation:**
- **Purpose:** Finds the first unset (zero) bit in a bitmap, starting from the given address.
- **How does it work?**
  - **`cld`** → Clears the direction flag to process forward.
  - **Loop (`1:`) reads one `long` (4 bytes) at a time**:
    - **`lodsl`** → Loads a 32-bit value from the address `addr` into `EAX`.
    - **`notl %%eax`** → Inverts bits in `EAX` (so zero bits become ones, helping find them).
    - **`bsfl %%eax,%%edx`** → Finds the first set bit (originally zero) in `EAX`, storing position in `EDX`.
    - **If no bit is found (`je 2f`)**, it moves to the next 32-bit chunk.
  - **Loop increments bit counter (`addl %%edx,%%ecx`)**.
  - **If all bits in `EAX` are 1s**, it moves to the next word (`addl $32, %%ecx`) and repeats.
  - **Stops when `ecx >= 8192` (no free bits found).**
- **Why is this needed?**
  - Optimized search for free blocks in memory.
  - Uses **fast bit scanning** to improve performance.

---

### **5. Freeing a Block (`free_block`)**

int free_block(int dev, int block) {
    struct super_block * sb;
    struct buffer_head * bh;

- **Declares pointers for `super_block` and `buffer_head` structures**, used to manage the filesystem.


    if (!(sb = get_super(dev)))
        panic("trying to free block on nonexistent device");

- **Retrieves the superblock for the given device (`dev`)**.
- If the device doesn’t exist, it triggers a **panic**.


    if (block < sb->s_firstdatazone || block >= sb->s_nzones)
        panic("trying to free block not in datazone");

- **Ensures the block number is within valid datazone limits**.


    bh = get_hash_table(dev,block);
    if (bh) {
        if (bh->b_count > 1) {
            brelse(bh);
            return 0;
        }

- **Fetches the buffer head for the block**.
- If the buffer exists and **is still referenced (`b_count > 1`)**, releases it and returns.


        bh->b_dirt=0;
        bh->b_uptodate=0;
        if (bh->b_count)
            brelse(bh);
    }

- **Marks the buffer as clean (`b_dirt=0`) and invalid (`b_uptodate=0`)**.
- Releases the buffer if it’s still in use.


    block -= sb->s_firstdatazone - 1;

- **Adjusts block index to align with bitmap**.

    if (clear_bit(block&8191, sb->s_zmap[block/8192]->b_data)) {
        printk("block (%04x:%d) ", dev, block + sb->s_firstdatazone - 1);
        printk("free_block: bit already cleared\n");
    }

- **Clears the bit corresponding to the block in the zone bitmap**.
- If the bit was **already cleared**, prints a warning.


    sb->s_zmap[block/8192]->b_dirt = 1;
    return 1;
}

- **Marks the zone bitmap as dirty (`b_dirt=1`)**, indicating a change.
- Returns **1**, indicating success.

#### **Why is this needed?**
- Ensures **blocks are properly freed**.
- Prevents **double freeing** by checking if the bit was already cleared.
- Keeps **bitmap data consistent**, marking changes as dirty for syncing.

---
