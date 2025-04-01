# Linux Scheduler Header File (sched.h)

This header file defines the core scheduling mechanisms in the Linux kernel. It establishes the process management structures, task states, and scheduling functions that enable multitasking in the operating system.

## System Constants and Configuration

The header begins by defining fundamental system parameters that govern process execution:


#define HZ 100                   /* System timer frequency (100 ticks per second) */
#define NR_TASKS 64              /* Maximum number of processes in the system */
#define TASK_SIZE 0x04000000     /* Memory size allocated to each process (64MB) */
#define LIBRARY_SIZE 0x00400000  /* Size reserved for shared libraries (4MB) */


These constants establish the foundation for memory management and process scheduling. The system performs several validation checks to ensure the memory layout is correct:


/* Ensure TASK_SIZE is aligned to 4MB boundaries for proper memory management */
#if (TASK_SIZE & 0x3fffff)
#error "TASK_SIZE must be multiple of 4M"
#endif

/* Verify library size is properly aligned */
#if (LIBRARY_SIZE & 0x3fffff)
#error "LIBRARY_SIZE must be a multiple of 4M"
#endif

/* Prevent libraries from consuming too much task memory */
#if (LIBRARY_SIZE >= (TASK_SIZE/2))
#error "LIBRARY_SIZE too damn big!"
#endif

/* Ensure total memory usage for all tasks equals 4GB (complete address space) */
#if (((TASK_SIZE>>16)*NR_TASKS) != 0x10000)
#error "TASK_SIZE*NR_TASKS must be 4GB"
#endif


These compile-time checks prevent configuration errors that could lead to unpredictable system behavior. For example, ensuring TASK_SIZE is a multiple of 4MB aligns with memory management unit requirements, while the task count check ensures the memory layout can utilize the entire 32-bit address space.

## Time-Related Macros

The kernel converts between various time representations:


#define CT_TO_SECS(x)   ((x) / HZ)                    /* Convert clock ticks to seconds */
#define CT_TO_USECS(x)  (((x) % HZ) * 1000000/HZ)     /* Convert remainder ticks to microseconds */


## Process States

Task states are defined using clear, meaningful constants:


#define TASK_RUNNING        0    /* Process is executing or ready to execute */
#define TASK_INTERRUPTIBLE  1    /* Process is sleeping but can be awakened by signals */
#define TASK_UNINTERRUPTIBLE 2   /* Process is sleeping and won't respond to signals */
#define TASK_ZOMBIE         3    /* Process execution is terminated, but parent hasn't collected status */
#define TASK_STOPPED        4    /* Process execution has been stopped by a signal */


These states form the foundation of the process lifecycle management, enabling the scheduler to track which processes are eligible to run.

## Architecture-Specific Data Structures

### FPU State Storage

The `i387_struct` structure preserves the state of the x87 FPU (Floating-Point Unit) during context switches:


struct i387_struct {
    long    cwd;                   /* Control Word */
    long    swd;                   /* Status Word */
    long    twd;                   /* Tag Word */
    long    fip;                   /* FPU Instruction Pointer */
    long    fcs;                   /* FPU Code Segment */
    long    foo;                   /* FPU Operand Offset */
    long    fos;                   /* FPU Operand Segment */
    long    st_space[20];          /* 8*10 bytes for each FP-reg = 80 bytes */
};


This structure allows the kernel to preserve floating-point calculations when switching between tasks.

### Task State Segment

The `tss_struct` represents the Task State Segment, an x86-specific data structure used for context switching:


struct tss_struct {
    long    back_link;     /* 16 high bits zero - previous TSS in task chain */
    long    esp0;          /* Stack pointer for privilege level 0 (kernel mode) */
    long    ss0;           /* Stack segment for privilege level 0 */
    /* Additional privilege level stacks */
    long    esp1;
    long    ss1;
    long    esp2;
    long    ss2;
    long    cr3;           /* Page directory base register - memory management */
    long    eip;           /* Instruction pointer */
    long    eflags;        /* CPU flags register */
    /* General purpose registers */
    long    eax,ecx,edx,ebx;
    long    esp;
    long    ebp;
    long    esi;
    long    edi;
    /* Segment registers */
    long    es, cs, ss, ds, fs, gs;
    long    ldt;           /* Local Descriptor Table */
    long    trace_bitmap;  /* Debug trace control */
    struct i387_struct i387;  /* FPU state */
};


The hardware uses this structure during context switches to restore register values for each task.

## Process Control Block

The `task_struct` is the cornerstone of process management, containing all information the kernel needs to manage a process:


struct task_struct {
    /* Core scheduling state */
    long state;            /* Process state: running, waiting, stopped, etc. */
    long counter;          /* Time quantum remaining for this process */
    long priority;         /* Base scheduling priority */
    
    /* Signal handling */
    long signal;           /* Bitmap of pending signals */
    struct sigaction sigaction[32];  /* Signal handlers */
    long blocked;          /* Bitmap of masked (blocked) signals */
    
    /* Process exit information */
    int exit_code;         /* Exit status to be reported to parent */
    
    /* Memory layout information */
    unsigned long start_code, end_code;  /* Code segment boundaries */
    unsigned long end_data, brk;         /* Data segment and heap boundaries */
    unsigned long start_stack;           /* Stack starting address */
    
    /* Process relationships */
    long pid, pgrp, session, leader;     /* Process ID and process groups */
    int groups[NGROUPS];                 /* Supplementary group IDs */
    
    /* Process tree linkage */
    struct task_struct *p_pptr;          /* Pointer to parent process */
    struct task_struct *p_cptr;          /* Pointer to youngest child */
    struct task_struct *p_ysptr;         /* Pointer to younger sibling */
    struct task_struct *p_osptr;         /* Pointer to older sibling */
    
    /* User and group IDs */
    unsigned short uid, euid, suid;      /* Real, effective, saved user ID */
    unsigned short gid, egid, sgid;      /* Real, effective, saved group ID */
    
    /* Timing information */
    unsigned long timeout, alarm;        /* Timeout and alarm values */
    long utime, stime;                   /* User and system time */
    long cutime, cstime;                 /* Child user and system time */
    long start_time;                     /* Process start time */
    
    /* Resource limits */
    struct rlimit rlim[RLIM_NLIMITS];
    
    /* Process flags and state */
    unsigned int flags;
    unsigned short used_math;            /* Process has used FPU */
    
    /* File system information */
    int tty;                             /* Controlling terminal */
    unsigned short umask;                /* File creation mask */
    struct m_inode *pwd;                 /* Current working directory */
    struct m_inode *root;                /* Root directory */
    struct m_inode *executable;          /* Executable file */
    struct m_inode *library;             /* Shared library file */
    unsigned long close_on_exec;         /* Bitmap of FDs to close on exec() */
    struct file *filp[NR_OPEN];          /* Open file table */
    
    /* Memory management */
    struct desc_struct ldt[3];           /* Local Descriptor Table entries */
    
    /* Hardware context */
    struct tss_struct tss;               /* Task State Segment for context switching */
};


This comprehensive structure tracks every aspect of a process's execution context, including its:
- Current execution state
- CPU scheduling information
- Memory mappings
- File access details
- Signal handling settings
- Relationships with other processes

## Process Flags

Process flags modify behavior for specific processes:


/*
 * Per process flags
 */
#define PF_ALIGNWARN    0x00000001    /* Print alignment warning messages (486-specific) */


## Task Initialization

The `INIT_TASK` macro defines the initial task structure (process 0, the kernel idle process):


#define INIT_TASK \
/* Initial state, priority, etc. */ { 0,15,15, \
/* signals */   0,{{},},0, \
/* Initial memory values */ 0,0,0,0,0,0, \
/* Initial process IDs */ 0,0,0,0, \
/* Group membership */ {NOGROUP,}, \
/* Process relationships */ &init_task.task,0,0,0, \
/* Initial UID/GID values */ 0,0,0,0,0,0, \
/* Initial timing values */ 0,0,0,0,0,0,0, \
/* Resource limits */ { {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff},  \
                       {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff}, \
                       {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff}}, \
/* Process flags */ 0, \
/* FPU usage */ 0, \
/* File system info */ -1,0022,NULL,NULL,NULL,NULL,0, \
/* File descriptors */ {NULL,}, \
    { \
        {0,0}, \
/* LDT entries */ {0x9f,0xc0fa00}, \
                  {0x9f,0xc0f200}, \
    }, \
/* Task State Segment */ {0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&pg_dir,\
     0,0,0,0,0,0,0,0, \
     0,0,0x17,0x17,0x17,0x17,0x17,0x17, \
     _LDT(0),0x80000000, \
        {} \
    }, \
}


This initializes process 0 with default values, establishing the root of the process tree.

## Global Variables

The header declares several key global variables:


extern struct task_struct *task[NR_TASKS];      /* Process table - array of all processes */
extern struct task_struct *last_task_used_math;  /* Last process that used the FPU */
extern struct task_struct *current;             /* Currently executing process */
extern unsigned long volatile jiffies;          /* System uptime in timer ticks */
extern unsigned long startup_time;              /* System boot time in

