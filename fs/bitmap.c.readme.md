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

### **6. Allocating a New Block (`new_block`)**

int new_block(int dev)

#### **Explanation:**
- **Purpose:** Allocates a new disk block in the bitmap.
- **How does it work?**
  - **Retrieve the Superblock:**

     if (!(sb = get_super(dev)))
        panic("trying to get new block from nonexistant device");

     - Fetches the filesystem's superblock.
     - If not found, it triggers a kernel panic.
  - **Search for a Free Block:**

     j = 8192;
     for (i = 0; i < 8; i++)
        if ((bh = sb->s_zmap[i]))
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;

     - Loops over **bitmap blocks** to find a free bit.
     - Uses `find_first_zero` to locate the first available block.
  - **Check If a Block Was Found:**

     if (i >= 8 || !bh || j >= 8192)
        return 0;

     - If no free block was found, return `0` (failure).
  - **Set the Bit to Mark It as Used:**

     if (set_bit(j, bh->b_data))
        panic("new_block: bit already set");

     - Marks the block as allocated.
     - If the bit was **already set**, it triggers a **panic** (should never happen).
  - **Adjust for Disk Offset:**

     j += i * 8192 + sb->s_firstdatazone - 1;
     if (j >= sb->s_nzones)
        return 0;

     - Converts **bitmap index** to **disk block number**.
     - If out of range, return `0` (failure).
  - **Fetch the Block and Initialize It:**

     if (!(bh = getblk(dev, j)))
        panic("new_block: cannot get block");
     if (bh->b_count != 1)
        panic("new block: count is != 1");
     clear_block(bh->b_data);

     - Fetches block from disk.
     - Ensures it’s not in use (`b_count == 1`).
     - Clears its contents using `clear_block`.
  - **Mark Block as Dirty and Release It:**

     bh->b_uptodate = 1;
     bh->b_dirt = 1;
     brelse(bh);
     return j;

     - Marks block as **dirty** (modified in memory but not written to disk).
     - Releases the buffer.
     - Returns the new block number.

---

### **7. Freeing an Inode (`free_inode`)**

void free_inode(struct m_inode * inode)

#### **Explanation:**
- **Purpose:** Frees an inode by clearing its corresponding bit in the inode bitmap.
- **How does it work?**
  - **Check if the inode is NULL:**

     if (!inode)
        return;

     - If `inode` is `NULL`, return immediately.
  - **Check if the inode is associated with a device:**

     if (!inode->i_dev) {
        memset(inode,0,sizeof(*inode));
        return;
     }

     - If the inode is not linked to a device, clear its memory and return.
  - **Ensure inode is not in use:**

     if (inode->i_count>1) {
        printk("trying to free inode with count=%d\n", inode->i_count);
        panic("free_inode");
     }

     - If `i_count` is greater than 1, it means the inode is still referenced elsewhere.
     - A kernel panic is triggered if an active inode is attempted to be freed.
  - **Ensure inode has no links:**

     if (inode->i_nlinks)
        panic("trying to free inode with links");

     - If `i_nlinks > 0`, the inode is still linked to files, and freeing it is incorrect.
  - **Retrieve the superblock and validate inode number:**

     if (!(sb = get_super(inode->i_dev)))
        panic("trying to free inode on nonexistent device");
     if (inode->i_num < 1 || inode->i_num > sb->s_ninodes)
        panic("trying to free inode 0 or nonexistent inode");

     - Fetches the **superblock** to ensure valid inode data.
     - If the inode number is out of range, it triggers a **panic**.
  - **Retrieve inode bitmap and clear the inode bit:**

     if (!(bh=sb->s_imap[inode->i_num>>13]))
        panic("nonexistent imap in superblock");
     if (clear_bit(inode->i_num&8191,bh->b_data))
        printk("free_inode: bit already cleared.\n");

     - Retrieves the **inode bitmap block**.
     - Clears the bit corresponding to the inode.
     - If the bit was already cleared, a message is printed.
  - **Mark inode bitmap as dirty and clear inode memory:**

     bh->b_dirt = 1;
     memset(inode,0,sizeof(*inode));

     - Marks the bitmap **dirty** (needs to be written to disk).
     - Clears the inode structure.

---

### **4. Creating a New Inode (`new_inode`)**

struct m_inode * new_inode(int dev)

#### **Explanation:**
- **Purpose:** Allocates a new inode by finding a free entry in the inode bitmap.
- **How does it work?**
  - **Declare required variables:**

     struct m_inode * inode;
     struct super_block * sb;
     struct buffer_head * bh;
     int i,j;

     - `inode` → Pointer to the newly allocated inode.
     - `sb` → Pointer to the superblock of the filesystem.
     - `bh` → Buffer head for inode bitmap manipulation.
     - `i, j` → Loop counters and indices for bitmap search.
  - **Get an empty inode structure:**

     if (!(inode=get_empty_inode()))
        return NULL;

     - Retrieves an unused inode from memory.
     - If no free inode is available, returns `NULL`.
  - **Retrieve the superblock:**

     if (!(sb = get_super(dev)))
        panic("new_inode with unknown device");

     - Fetches the superblock corresponding to the given device `dev`.
     - If the superblock does not exist, triggers a **kernel panic**.
  - **Find a free inode in the bitmap:**

     j = 8192;
     for (i=0 ; i<8 ; i++)
        if ((bh=sb->s_imap[i]))
           if ((j=find_first_zero(bh->b_data))<8192)
              break;

     - Loops through the inode bitmap to locate the first **free** inode.
     - Uses `find_first_zero(bh->b_data)` to search within each block.
     - Stops when a **free inode** is found or all 8 blocks are checked.
  - **Ensure a valid inode index:**

     if (!bh || j >= 8192 || j+i*8192 > sb->s_ninodes) {
        iput(inode);
        return NULL;
     }

     - If no free inode is found or index exceeds the maximum inode count:
       - Releases the `inode` reference using `iput()`.
       - Returns `NULL`.
  - **Mark the inode as allocated in the bitmap:**

     if (set_bit(j,bh->b_data))
        panic("new_inode: bit already set");

     - Sets the corresponding bit in the inode bitmap.
     - If the bit was **already set**, a **kernel panic** occurs.
  - **Mark the bitmap block as dirty:**

     bh->b_dirt = 1;

     - Indicates that the **inode bitmap** has changed and must be written back to disk.
  - **Initialize inode fields:**

     inode->i_count=1;
     inode->i_nlinks=1;
     inode->i_dev=dev;
     inode->i_uid=current->euid;
     inode->i_gid=current->egid;
     inode->i_dirt=1;
     inode->i_num = j + i*8192;
     inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;

     - **`i_count = 1`** → The inode is now active.
     - **`i_nlinks = 1`** → It has at least one link.
     - **`i_dev = dev`** → Assigns the device ID.
     - **`i_uid/i_gid`** → Inherits user and group ID from the current process.
     - **`i_dirt = 1`** → Marks the inode as modified.
     - **`i_num`** → Computes the absolute inode number.
     - **Timestamps (`i_mtime`, `i_atime`, `i_ctime`)** → Set to current system time.
  - **Return the newly allocated inode:**

     return inode;

     - Returns a pointer to the **allocated and initialized inode**.

---








