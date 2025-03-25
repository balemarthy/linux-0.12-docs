# Floppy Disk Controller Register Definitions

This header file (`fdreg.h`) defines constants and function prototypes for interacting with the floppy disk controller (FDC) in a Linux kernel. It provides a comprehensive reference for the register addresses, bit flags, and command values needed for low-level floppy disk operations.

## Overview

Floppy disk controllers were common in early personal computers, and this code defines the necessary constants to interact with the standard IBM PC-compatible floppy controllers. These definitions are primarily based on the "IBM Microcomputers: A Programmer's Handbook" by Sanches and Canton.

## External Functions

The file declares several external functions for floppy disk operations:


extern int ticks_to_floppy_on(unsigned int nr);
extern void floppy_on(unsigned int nr);
extern void floppy_off(unsigned int nr);
extern void floppy_select(unsigned int nr);
extern void floppy_deselect(unsigned int nr);


These functions handle basic floppy drive operations:

- **ticks_to_floppy_on**: Returns the number of clock ticks needed to wait for a floppy drive to power up
- **floppy_on**: Powers up the specified floppy drive
- **floppy_off**: Powers down the specified floppy drive
- **floppy_select**: Selects the specified floppy drive for operations
- **floppy_deselect**: Deselects the specified floppy drive

The `nr` parameter typically specifies which floppy drive to operate on (0 for A:, 1 for B:, etc.)

## FDC Register Addresses

The controller exposes several I/O ports for interaction:


#define FD_STATUS   0x3f4   /* Main Status Register (read) */
#define FD_DATA     0x3f5   /* Data Register (read/write) */
#define FD_DOR      0x3f2   /* Digital Output Register (write) */
#define FD_DIR      0x3f7   /* Digital Input Register (read) */
#define FD_DCR      0x3f7   /* Diskette Control Register (write) */


These registers are accessed via I/O ports in the x86 architecture:
- **FD_STATUS**: Used to check the status of the FDC
- **FD_DATA**: Used to transfer command parameters and results
- **FD_DOR**: Controls drive motors, drive selection, and reset
- **FD_DIR**: Reports the state of disk-change detection and other inputs
- **FD_DCR**: Controls data rate and other parameters (shares the same address as DIR)

## Status Register Bit Flags

### Main Status Register (FD_STATUS)


#define STATUS_BUSYMASK 0x0F    /* drive busy mask */
#define STATUS_BUSY     0x10    /* FDC busy */
#define STATUS_DMA      0x20    /* 0- DMA mode */
#define STATUS_DIR      0x40    /* 0- cpu->fdc */
#define STATUS_READY    0x80    /* Data reg ready */


These bits help determine the current state of the FDC:
- **STATUS_BUSYMASK**: Bits 0-3 indicate which drives are busy
- **STATUS_BUSY**: Set when the FDC is executing a command
- **STATUS_DMA**: Clear when the FDC is in DMA mode
- **STATUS_DIR**: Direction of data transfer (0 = CPU to FDC, 1 = FDC to CPU)
- **STATUS_READY**: Set when the data register is ready for I/O

### Status Register 0 (ST0)


#define ST0_DS      0x03    /* drive select mask */
#define ST0_HA      0x04    /* Head (Address) */
#define ST0_NR      0x08    /* Not Ready */
#define ST0_ECE     0x10    /* Equipment check error */
#define ST0_SE      0x20    /* Seek end */
#define ST0_INTR    0xC0    /* Interrupt code mask */


ST0 is returned after most commands and provides status information:
- **ST0_DS**: Bits 0-1 indicate which drive was used
- **ST0_HA**: Which head was active (0 or 1)
- **ST0_NR**: Set if the drive was not ready
- **ST0_ECE**: Equipment check error (usually indicates a hardware problem)
- **ST0_SE**: Set when a seek operation completes
- **ST0_INTR**: Bits 6-7 provide an interrupt code (normal/abnormal termination)

### Status Register 1 (ST1)


#define ST1_MAM     0x01    /* Missing Address Mark */
#define ST1_WP      0x02    /* Write Protect */
#define ST1_ND      0x04    /* No Data - unreadable */
#define ST1_OR      0x10    /* OverRun */
#define ST1_CRC     0x20    /* CRC error in data or addr */
#define ST1_EOC     0x80    /* End Of Cylinder */


ST1 provides additional error information after read/write operations:
- **ST1_MAM**: No address mark found on the track
- **ST1_WP**: Operation failed because the disk is write-protected
- **ST1_ND**: No valid data could be found
- **ST1_OR**: Data transfer couldn't keep up with the disk rotation
- **ST1_CRC**: Data integrity error detected
- **ST1_EOC**: Attempt to access beyond the end of a cylinder

### Status Register 2 (ST2)


#define ST2_MAM     0x01    /* Missing Address Mark (again) */
#define ST2_BC      0x02    /* Bad Cylinder */
#define ST2_SNS     0x04    /* Scan Not Satisfied */
#define ST2_SEH     0x08    /* Scan Equal Hit */
#define ST2_WC      0x10    /* Wrong Cylinder */
#define ST2_CRC     0x20    /* CRC error in data field */
#define ST2_CM      0x40    /* Control Mark = deleted */


ST2 provides even more detailed error information:
- **ST2_MAM**: Same as ST1_MAM (redundant check)
- **ST2_BC**: Cylinder ID in the sector header is bad
- **ST2_SNS**: Scan command condition not met
- **ST2_SEH**: Scan command equal condition found
- **ST2_WC**: Track accessed was not the expected one
- **ST2_CRC**: Data integrity error in the sector data
- **ST2_CM**: Deleted data mark encountered

### Status Register 3 (ST3)


#define ST3_HA      0x04    /* Head (Address) */
#define ST3_TZ      0x10    /* Track Zero signal (1=track 0) */
#define ST3_WP      0x40    /* Write Protect */


ST3 reports the drive state:
- **ST3_HA**: Current head select state
- **ST3_TZ**: Set when the drive is positioned at track 0
- **ST3_WP**: Set when the disk is write-protected

## FDC Commands


#define FD_RECALIBRATE  0x07    /* move to track 0 */
#define FD_SEEK         0x0F    /* seek track */
#define FD_READ         0xE6    /* read with MT, MFM, SKip deleted */
#define FD_WRITE        0xC5    /* write with MT, MFM */
#define FD_SENSEI       0x08    /* Sense Interrupt Status */
#define FD_SPECIFY      0x03    /* specify HUT etc */


These values represent FDC command opcodes:
- **FD_RECALIBRATE**: Moves the read/write head to track 0
- **FD_SEEK**: Positions the head at a specific track
- **FD_READ**: Reads data with specific options (Multi-Track, MFM encoding, Skip deleted data)
- **FD_WRITE**: Writes data with specific options (Multi-Track, MFM encoding)
- **FD_SENSEI**: Retrieves the status after an interrupt
- **FD_SPECIFY**: Sets timing parameters (Head Unload Time, etc.)

## DMA Commands


#define DMA_READ    0x46
#define DMA_WRITE   0x4A


These constants define DMA (Direct Memory Access) operations:
- **DMA_READ**: DMA mode for reading data from the floppy to memory
- **DMA_WRITE**: DMA mode for writing data from memory to the floppy

## Technical Implementation

The floppy controller typically uses DMA for data transfers to ensure proper timing. When reading or writing sectors:

1. The CPU sets up the DMA controller with the memory address and count
2. The CPU sends the appropriate command to the FDC
3. The FDC and DMA controller handle the data transfer directly
4. Upon completion, the FDC generates an interrupt
5. The driver reads the status registers to determine if the operation was successful

This approach minimizes CPU overhead during the actual data transfer, which was crucial for maintaining proper timing with these mechanical devices.

## Historical Context

Understanding these register definitions is crucial for low-level floppy disk driver development in operating systems like Linux. While floppy disks are largely obsolete in modern systems, this code represents an important part of computing history and provides insight into how early storage devices were controlled at a hardware level.

