# AT Hard Disk Controller Header File Documentation

## Overview

This header file (`hdreg.h`) defines the low-level interface constants and structures for interacting with AT-compatible hard disk controllers. It provides register definitions, status and command bit flags, and data structures necessary for direct programmatic control of hard disk operations in a Linux/Unix environment.

## Register Definitions

The AT hard disk controller uses I/O ports to communicate with the CPU. These registers are accessed through specific I/O addresses that serve different purposes:


#define HD_DATA     0x1f0   /* Data register (doubles as _CTL when writing) */
#define HD_ERROR    0x1f1   /* Error register (read) / Write Precompensation (write) */
#define HD_NSECTOR  0x1f2   /* Number of sectors to read/write */
#define HD_SECTOR   0x1f3   /* Starting sector number */
#define HD_LCYL     0x1f4   /* Low byte of cylinder number */
#define HD_HCYL     0x1f5   /* High byte of cylinder number */
#define HD_CURRENT  0x1f6   /* Drive/Head select (101dhhhh, d=drive, hhhh=head) */
#define HD_STATUS   0x1f7   /* Status register (read) / Command register (write) */
#define HD_PRECOMP  HD_ERROR    /* Write precompensation shares address with error register */
#define HD_COMMAND  HD_STATUS   /* Command register shares address with status register */

#define HD_CMD      0x3f6   /* Alternate status/device control */


These definitions enable programmatic access to the physical controller registers. The I/O addresses are standard for AT-compatible controllers as referenced in the IBM AT BIOS specifications.

## Status Register Bit Flags

The status register (accessible at address `0x1f7` when reading) provides real-time information about the disk controller state:


/* Bits of HD_STATUS */
#define ERR_STAT    0x01    /* Error occurred (check error register) */
#define INDEX_STAT  0x02    /* Index mark detected */
#define ECC_STAT    0x04    /* Error Correction Code applied successfully */
#define DRQ_STAT    0x08    /* Data Request - controller ready for data transfer */
#define SEEK_STAT   0x10    /* Seek operation completed */
#define WRERR_STAT  0x20    /* Write error occurred */
#define READY_STAT  0x40    /* Drive is ready (can accept commands) */
#define BUSY_STAT   0x80    /* Controller is busy executing a command */


These bit flags allow software to determine the disk controller's current state by reading the status register and testing specific bits. For example, checking `BUSY_STAT` before issuing a new command prevents command collision.

## Command Values

The command register (same address as status, `0x1f7`, but when writing) accepts these operation codes:


/* Values for HD_COMMAND */
#define WIN_RESTORE     0x10    /* Recalibrate drive (seek to track 0) */
#define WIN_READ        0x20    /* Read sectors */
#define WIN_WRITE       0x30    /* Write sectors */
#define WIN_VERIFY      0x40    /* Verify sectors (no data transfer) */
#define WIN_FORMAT      0x50    /* Format track */
#define WIN_INIT        0x60    /* Controller initialization */
#define WIN_SEEK        0x70    /* Seek to specified cylinder */
#define WIN_DIAGNOSE    0x90    /* Perform controller diagnostics */
#define WIN_SPECIFY     0x91    /* Set drive parameters */


These commands instruct the controller to perform specific operations. The naming prefix "WIN_" likely refers to the Winchester disk technology commonly used in early hard drives.

## Error Register Bit Flags

When an error occurs (indicated by `ERR_STAT` in the status register), the error register (`0x1f1`) provides details:


/* Bits for HD_ERROR */
#define MARK_ERR    0x01    /* Bad address mark detected */
#define TRK0_ERR    0x02    /* Track 0 not found during recalibration */
#define ABRT_ERR    0x04    /* Command aborted due to drive status error */
#define ID_ERR      0x10    /* ID field not found */
#define ECC_ERR     0x40    /* Uncorrectable ECC error */
#define BBD_ERR     0x80    /* Bad block detected */


These error codes help diagnose specific hardware or media issues during disk operations. Software can read this register after detecting an error condition to determine the appropriate response.

## Partition Structure

The header also defines a data structure for disk partitions:


struct partition {
    unsigned char boot_ind;     /* 0x80 indicates bootable (active) partition */
    unsigned char head;         /* Starting head */
    unsigned char sector;       /* Starting sector */
    unsigned char cyl;          /* Starting cylinder */
    unsigned char sys_ind;      /* System indicator (partition type) */
    unsigned char end_head;     /* Ending head */
    unsigned char end_sector;   /* Ending sector */
    unsigned char end_cyl;      /* Ending cylinder */
    unsigned int start_sect;    /* Starting sector (absolute, counting from 0) */
    unsigned int nr_sects;      /* Number of sectors in partition */
};


This structure matches the partition table format stored in the Master Boot Record (MBR) of disks. It contains both the CHS (Cylinder-Head-Sector) addressing information (legacy format) and the LBA (Logical Block Addressing) information (start_sect and nr_sects) for each partition.

## Implementation Notes

This header file is designed to be included in kernel-level code that needs direct access to disk hardware. The implementation follows these principles:

1. **Direct Hardware Interfacing**: The definitions provide a thin abstraction over hardware I/O ports, allowing for precise control of the disk controller.

2. **Consistent Naming Conventions**: Prefixes like `HD_` for registers and `WIN_` for commands create a logical grouping of related constants.

3. **Comprehensive Bit Flags**: By defining individual bit masks for status and error registers, the code that uses this header can perform precise bit-level operations.

4. **Memory Efficiency**: The partition structure is packed efficiently to match the on-disk format exactly, ensuring correct interpretation of disk data.

The header demonstrates low-level hardware programming practices typical in operating system development, where direct control of hardware is necessary for performance and functionality.

