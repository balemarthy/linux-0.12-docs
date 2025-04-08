# Linux Resource Control and Accounting Header

## Overview

This header file (`resource.h`) defines structures and constants for resource control and accounting in Linux systems. It provides mechanisms for tracking resource usage statistics and enforcing resource limits on processes, which are essential features for system stability, performance monitoring, and fair resource allocation in a multi-process operating system.

## Resource Usage Tracking

The header defines a structure for resource usage statistics based on BSD 4.3 Reno, enabling applications to monitor how system resources are being consumed.

### The `rusage` Structure


struct rusage {
    struct timeval ru_utime;    /* user time used */
    struct timeval ru_stime;    /* system time used */
    long    ru_maxrss;          /* maximum resident set size */
    long    ru_ixrss;           /* integral shared memory size */
    long    ru_idrss;           /* integral unshared data size */
    long    ru_isrss;           /* integral unshared stack size */
    long    ru_minflt;          /* page reclaims */
    long    ru_majflt;          /* page faults */
    long    ru_nswap;           /* swaps */
    long    ru_inblock;         /* block input operations */
    long    ru_oublock;         /* block output operations */
    long    ru_msgsnd;          /* messages sent */
    long    ru_msgrcv;          /* messages received */
    long    ru_nsignals;        /* signals received */
    long    ru_nvcsw;           /* voluntary context switches */
    long    ru_nivcsw;          /* involuntary context switches */
};


This comprehensive structure tracks various aspects of process resource usage:

- **Time Metrics**: Both user and system time consumed
- **Memory Usage**: Tracking of resident set size, shared memory, and unshared memory segments
- **Paging Activity**: Monitors minor and major page faults
- **I/O Operations**: Tracks block input/output operations
- **IPC Activity**: Records message sending and receiving
- **Context Switching**: Distinguishes between voluntary and involuntary context switches

### Usage Identifiers


#define RUSAGE_SELF      0
#define RUSAGE_CHILDREN  -1


These constants allow for specifying whether to retrieve resource usage statistics for:

- **RUSAGE_SELF**: The calling process itself
- **RUSAGE_CHILDREN**: All children of the calling process that have terminated and been waited for

These identifiers are typically used with the `getrusage()` system call (not defined in this header) to specify which resource usage metrics to retrieve.

## Resource Limits Control

The header also defines mechanisms for setting and enforcing resource limits on processes, which is critical for system stability and fair resource allocation.

### Resource Types


#define RLIMIT_CPU    0        /* CPU time in ms */
#define RLIMIT_FSIZE  1        /* Maximum filesize */
#define RLIMIT_DATA   2        /* max data size */
#define RLIMIT_STACK  3        /* max stack size */
#define RLIMIT_CORE   4        /* max core file size */
#define RLIMIT_RSS    5        /* max resident set size */

#ifdef notdef
#define RLIMIT_MEMLOCK  6      /* max locked-in-memory address space*/
#define RLIMIT_NPROC    7      /* max number of processes */
#define RLIMIT_OFILE    8      /* max number of open files */
#endif


The header defines specific resource types that can be limited:

- **RLIMIT_CPU**: Maximum CPU time a process can consume
- **RLIMIT_FSIZE**: Maximum file size a process can create
- **RLIMIT_DATA**: Maximum size of the data segment for a process
- **RLIMIT_STACK**: Maximum size of the stack segment
- **RLIMIT_CORE**: Maximum size of core dump files
- **RLIMIT_RSS**: Maximum resident set size (physical memory consumption)

Additionally, there are commented-out definitions for future expansion:
- Memory locking limits
- Process count limits
- Open file limits

### Limit Management Constants


#define RLIM_NLIMITS    6
#define RLIM_INFINITY   0x7fffffff


These constants define:
- **RLIM_NLIMITS**: The number of resource limits currently implemented
- **RLIM_INFINITY**: A special value representing an unlimited resource quota

### Resource Limits Structure


struct rlimit {
    int rlim_cur;    /* Current (soft) limit */
    int rlim_max;    /* Hard limit */
};


The `rlimit` structure enables the dual-threshold resource limiting mechanism:

- **rlim_cur**: The current (soft) limit that the kernel enforces for the resource
- **rlim_max**: The maximum (hard) limit to which the soft limit can be raised

This two-tier approach allows for flexible resource management where processes can adjust their own limits up to the hard ceiling defined by administrators.

## Architectural Considerations

This header implements several important architectural principles:

1. **Forward Compatibility**: The structure includes fields that weren't fully implemented at creation time to ensure future compatibility with applications using this interface.

2. **Clear Separation of Concerns**: 
   - Resource usage tracking (what has been used)
   - Resource limiting (what can be used)

3. **Hierarchical Control**: The ability to monitor both a process itself and its children enables hierarchical resource accounting.

4. **Dual-threshold Limits**: The soft/hard limit separation allows processes some autonomy in resource management while maintaining system-wide control.

---

This header file provides the foundation for both observability (through resource usage statistics) and control (through resource limits) of process resource consumption, which are fundamental capabilities for operating system resource management.

