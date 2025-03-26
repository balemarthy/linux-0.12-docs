# Linux Filesystem Structures (`fs.h`)

This header file defines the core data structures and functions used in the Linux filesystem implementation. It serves as the foundation for the filesystem layer, defining structures like inodes, file tables, buffer caches, and the interfaces for interacting with storage devices.

## Core File System Architecture

The Linux filesystem is built around several key abstractions:

1. **Inodes** - The basic units that represent files and directories
2. **Buffer Cache** - In-memory representation of disk blocks for performance
3. **Super Blocks** - Metadata about each mounted filesystem
4. **File Tables** - Track open files across the system

## Device Numbering System

Linux uses a device numbering system similar to the Minix filesystem, enabling compatibility. Devices are identified by major and minor numbers:


/* Device types by major number */
// 0 - unused (nodev)
// 1 - /dev/mem     (memory access)
// 2 - /dev/fd      (floppy disk)
// 3 - /dev/hd      (hard disk)
// 4 - /dev/ttyx    (terminal devices)
// 5 - /dev/tty     (controlling terminal)
// 6 - /dev/lp      (line printer)
// 7 - unnamed pipes (inter-process communication)


Seekable devices (those that support random access) are defined as:


#define IS_SEEKABLE(x) ((x)>=1 && (x)<=3)  // memory, floppy, and hard disk


Device numbers are split into major and minor components:


#define MAJOR(a) (((unsigned)(a))>>8)  // Extract the major device number (device type)
#define MINOR(a) ((a)&0xff)           // Extract the minor device number (specific instance)


## I/O Operations

The system defines four basic I/O operations:


#define READ 0      // Regular synchronous read
#define WRITE 1     // Regular synchronous write
#define READA 2     // Read-ahead - non-blocking prefetch
#define WRITEA 3    // Write-ahead - optimized writing


## Buffer Cache System

The buffer cache is a critical performance component that caches disk blocks in memory:


struct buffer_head {
    char * b_data;                  // Pointer to the actual data block (1024 bytes)
    unsigned long b_blocknr;        // Physical block number on device
    unsigned short b_dev;           // Device identifier (0 = free/unused)
    unsigned char b_uptodate;       // Is the buffer content valid/current?
    unsigned char b_dirt;           // Is the buffer modified (dirty)?
    unsigned char b_count;          // Reference count - number of users
    unsigned char b_lock;           // Is the buffer currently locked?
    struct task_struct * b_wait;    // Processes waiting for this buffer
    struct buffer_head * b_prev;    // Previous buffer in hash list
    struct buffer_head * b_next;    // Next buffer in hash list
    struct buffer_head * b_prev_free; // Previous buffer in free list
    struct buffer_head * b_next_free; // Next buffer in free list
};


The buffer system is initialized with:


void buffer_init(long buffer_end);  // Set up the buffer cache system


## Inode Structures

Linux defines two types of inodes:

### Disk Inode Structure
This is how inodes are stored on disk:


struct d_inode {
    unsigned short i_mode;     // File type and permissions
    unsigned short i_uid;      // Owner's user ID
    unsigned long i_size;      // File size in bytes
    unsigned long i_time;      // Modification time
    unsigned char i_gid;       // Owner's group ID
    unsigned char i_nlinks;    // Number of hard links to the file
    unsigned short i_zone[9];  // Zone (block) numbers for file data
};


### Memory Inode Structure
This extended structure is used while the inode is in memory:


struct m_inode {
    /* Fields also stored on disk */
    unsigned short i_mode;     // File type and permissions
    unsigned short i_uid;      // Owner's user ID
    unsigned long i_size;      // File size in bytes
    unsigned long i_mtime;     // Modification time
    unsigned char i_gid;       // Owner's group ID
    unsigned char i_nlinks;    // Number of hard links to the file
    unsigned short i_zone[9];  // Zone (block) numbers for file data
    
    /* Memory-only fields */
    struct task_struct * i_wait;    // Processes waiting on inode
    struct task_struct * i_wait2;   // Used for pipes
    unsigned long i_atime;          // Access time
    unsigned long i_ctime;          // Creation time
    unsigned short i_dev;           // Device number
    unsigned short i_num;           // Inode number
    unsigned short i_count;         // Reference count
    unsigned char i_lock;           // Is the inode locked?
    unsigned char i_dirt;           // Has the inode been modified?
    unsigned char i_pipe;           // Is this a pipe?
    unsigned char i_mount;          // Is this a mount point?
    unsigned char i_seek;           // Is someone seeking on this file?
    unsigned char i_update;         // Update time after i/o?
};


## Pipe Implementation

Pipes are implemented using inodes with special macros to manipulate their behavior:


#define PIPE_READ_WAIT(inode) ((inode).i_wait)         // Processes waiting to read
#define PIPE_WRITE_WAIT(inode) ((inode).i_wait2)       // Processes waiting to write
#define PIPE_HEAD(inode) ((inode).i_zone[0])           // Write position in pipe
#define PIPE_TAIL(inode) ((inode).i_zone[1])           // Read position in pipe
#define PIPE_SIZE(inode) ((PIPE_HEAD(inode)-PIPE_TAIL(inode))&(PAGE_SIZE-1))  // Data in pipe
#define PIPE_EMPTY(inode) (PIPE_HEAD(inode)==PIPE_TAIL(inode))  // Is pipe empty?
#define PIPE_FULL(inode) (PIPE_SIZE(inode)==(PAGE_SIZE-1))      // Is pipe full?


## File Representation

Open files are tracked using the file structure:


struct file {
    unsigned short f_mode;        // Access mode (read/write)
    unsigned short f_flags;       // File flags (O_APPEND, etc.)
    unsigned short f_count;       // Reference count
    struct m_inode * f_inode;     // Pointer to the inode for this file
    off_t f_pos;                  // Current file position (seek pointer)
};


## Filesystem Superblock

The superblock contains metadata about the entire filesystem:

### Disk Superblock
This is how the superblock is stored on disk:


struct d_super_block {
    unsigned short s_ninodes;        // Number of inodes
    unsigned short s_nzones;         // Number of zones (blocks)
    unsigned short s_imap_blocks;    // Number of inode bitmap blocks
    unsigned short s_zmap_blocks;    // Number of zone bitmap blocks
    unsigned short s_firstdatazone;  // First data zone
    unsigned short s_log_zone_size;  // Log2 of bytes per zone
    unsigned long s_max_size;        // Maximum file size
    unsigned short s_magic;          // Magic number to identify filesystem type
};


### Memory Superblock
Extended structure used while the filesystem is mounted:


struct super_block {
    /* Fields also stored on disk */
    unsigned short s_ninodes;
    unsigned short s_nzones;
    unsigned short s_imap_blocks;
    unsigned short s_zmap_blocks;
    unsigned short s_firstdatazone;
    unsigned short s_log_zone_size;
    unsigned long s_max_size;
    unsigned short s_magic;
    
    /* Memory-only fields */
    struct buffer_head * s_imap[8];  // Inode bitmap in memory
    struct buffer_head * s_zmap[8];  // Zone bitmap in memory
    unsigned short s_dev;            // Device number
    struct m_inode * s_isup;         // Pointer to inode of this superblock
    struct m_inode * s_imount;       // Inode mounted on
    unsigned long s_time;            // Modification time
    struct task_struct * s_wait;     // Processes waiting on superblock
    unsigned char s_lock;            // Is superblock locked?
    unsigned char s_rd_only;         // Is filesystem read-only?
    unsigned char s_dirt;            // Has superblock been modified?
};


## Directory Structure

Directories are represented as a collection of entries:


struct dir_entry {
    unsigned short inode;      // Inode number of the file
    char name[NAME_LEN];       // Filename (limited to NAME_LEN characters)
};


## System Limitations and Constants


#define NAME_LEN 14             // Maximum filename length
#define ROOT_INO 1              // Inode number of the root directory

#define I_MAP_SLOTS 8           // Maximum inode bitmap blocks
#define Z_MAP_SLOTS 8           // Maximum zone bitmap blocks
#define SUPER_MAGIC 0x137F      // Magic number for filesystem identification

#define NR_OPEN 20              // Maximum open files per process
#define NR_INODE 64             // Maximum inodes in memory
#define NR_FILE 64              // Maximum file structures
#define NR_SUPER 8              // Maximum mounted filesystems
#define NR_HASH 307             // Size of buffer hash table
#define NR_BUFFERS nr_buffers   // Number of buffer heads
#define BLOCK_SIZE 1024         // Size of a block in bytes
#define BLOCK_SIZE_BITS 10      // Log2 of BLOCK_SIZE


These constants define fundamental limits of the filesystem implementation.

## Key Filesystem Functions

The header declares numerous functions for filesystem operations:

### Buffer Management

// Get a buffer from the hash table or allocate a new one
struct buffer_head * getblk(int dev, int block);
// Read a block from disk into the buffer cache
struct buffer_head * bread(int dev, int block);
// Release a buffer that's no longer needed
void brelse(struct buffer_head * buf);
// Initiate low-level read/write operations on blocks
void ll_rw_block(int rw, struct buffer_head * bh);


### Inode Operations

// Find the inode for a given path
struct m_inode * namei(const char * pathname);
// Find inode without following symbolic links
struct m_inode * lnamei(const char * pathname);
// Get an inode from disk
struct m_inode * iget(int dev, int nr);
// Release an inode when no longer needed
void iput(struct m_inode * inode);
// Get a new empty inode
struct m_inode * get_empty_inode(void);
// Wait for operations on an inode to

