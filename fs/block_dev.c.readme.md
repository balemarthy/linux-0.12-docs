# README - Linux 0.12 `block_dev.c`

## Overview

The `block_dev.c` file is responsible for handling **low-level block device operations** in Linux 0.12. It provides functions for:

- **Reading data** from a block device (`block_read`).
- **Writing data** to a block device (`block_write`).

These functions operate **directly on disk blocks** and manage buffered I/O.

---

## **Functional Breakdown (Line-by-Line)**

### **1. Writing Data to a Block Device (`block_write`)**
```c
int block_write(int dev, long * pos, char * buf, int count)
```
#### **Explanation:**
- **Purpose:** Writes `count` bytes from `buf` to the block device `dev` at the position `pos`.
- **Parameters:**
  - `dev` → Block device identifier.
  - `pos` → Pointer to the file offset (starting position).
  - `buf` → Pointer to the user-space buffer containing data.
  - `count` → Number of bytes to write.
- **Returns:**
  - Number of bytes successfully written.
  - `-EIO` (I/O error) if the write fails.

#### **Step-by-Step Execution:**
1. **Calculate block number and offset in block:**
   ```c
   int block = *pos >> BLOCK_SIZE_BITS;
   int offset = *pos & (BLOCK_SIZE-1);
   ```
   - **`block`** → Determines which disk block the offset falls into.
   - **`offset`** → Determines the byte position inside that block.

2. **Initialize tracking variables:**
   ```c
   int chars;
   int written = 0;
   int size;
   struct buffer_head * bh;
   register char * p;
   ```
   - `chars` → Number of bytes that can be written in the current block.
   - `written` → Tracks total bytes written.
   - `size` → Holds the total number of blocks for the device.
   - `bh` → Buffer for block caching.
   - `p` → Pointer for byte manipulation.

3. **Check total device size:**
   ```c
   if (blk_size[MAJOR(dev)])
       size = blk_size[MAJOR(dev)][MINOR(dev)];
   else
       size = 0x7fffffff;
   ```
   - **Retrieves the device size** (if known) or assumes a very large default (`0x7fffffff`).

4. **Loop through the blocks to write data:**
   ```c
   while (count>0) {
   ```
   - Continues writing **until all requested bytes are written**.

5. **Check if the block number is valid:**
   ```c
   if (block >= size)
       return written ? written : -EIO;
   ```
   - Ensures the write does not exceed the device limits.

6. **Determine how many bytes can be written to the current block:**
   ```c
   chars = BLOCK_SIZE - offset;
   if (chars > count)
       chars = count;
   ```
   - Limits `chars` to the number of bytes remaining in the block.

7. **Get the block buffer:**
   ```c
   if (chars == BLOCK_SIZE)
       bh = getblk(dev, block);
   else
       bh = breada(dev, block, block+1, block+2, -1);
   ```
   - If writing a **full block**, use `getblk` (direct allocation).
   - Otherwise, use `breada` (buffered read-ahead).

8. **Handle allocation failure:**
   ```c
   if (!bh)
       return written ? written : -EIO;
   ```
   - If no buffer is available, return an **I/O error**.

9. **Write data to the block:**
   ```c
   p = offset + bh->b_data;
   offset = 0;
   *pos += chars;
   written += chars;
   count -= chars;
   while (chars-- > 0)
       *(p++) = get_fs_byte(buf++);
   ```
   - Copies bytes from `buf` into the block buffer.

10. **Mark buffer as dirty and release it:**
    ```c
    bh->b_dirt = 1;
    brelse(bh);
    ```
    - Marks the buffer for writing to disk.
    - Releases it after writing.

---

### **2. Reading Data from a Block Device (`block_read`)**
```c
int block_read(int dev, unsigned long * pos, char * buf, int count)
```
#### **Explanation:**
- **Purpose:** Reads `count` bytes from the block device `dev` at `pos` into `buf`.
- **Returns:**
  - Number of bytes read.
  - `-EIO` (I/O error) if the read fails.

#### **Step-by-Step Execution:**
1. **Determine block number and offset in block:**
   ```c
   int block = *pos >> BLOCK_SIZE_BITS;
   int offset = *pos & (BLOCK_SIZE-1);
   ```

2. **Initialize tracking variables:**
   ```c
   int chars;
   int size;
   int read = 0;
   struct buffer_head * bh;
   register char * p;
   ```

3. **Check device size:**
   ```c
   if (blk_size[MAJOR(dev)])
       size = blk_size[MAJOR(dev)][MINOR(dev)];
   else
       size = 0x7fffffff;
   ```

4. **Loop through the blocks to read data:**
   ```c
   while (count > 0) {
   ```

5. **Ensure block number is valid:**
   ```c
   if (block >= size)
       return read ? read : -EIO;
   ```

6. **Determine bytes available in the block:**
   ```c
   chars = BLOCK_SIZE - offset;
   if (chars > count)
       chars = count;
   ```

7. **Read block into buffer:**
   ```c
   if (!(bh = breada(dev, block, block+1, block+2, -1)))
       return read ? read : -EIO;
   ```

8. **Copy data from block buffer to user buffer:**
   ```c
   block++;
   p = offset + bh->b_data;
   offset = 0;
   *pos += chars;
   read += chars;
   count -= chars;
   while (chars-- > 0)
       put_fs_byte(*(p++), buf++);
   ```
   - Reads bytes from the block buffer into `buf`.

9. **Release buffer:**
   ```c
   brelse(bh);
   ```

10. **Return number of bytes read.**

---

## **Summary**
- **`block_write`** → Writes data to disk, ensuring proper buffering and alignment.
- **`block_read`** → Reads data efficiently using block caching.
- **Error handling ensures safe disk operations.**

This file is **crucial** for managing **disk-based file I/O** in Linux 0.12.



