# a.out.h - The Traditional UNIX Object File Format Header

## Overview

This header file defines the structures and macros for working with the traditional UNIX a.out object file format. The a.out (assembler output) format was one of the earliest binary executable formats used in Unix systems and remains important for understanding Unix history and legacy systems.

## The Executable Header Structure

The core of the a.out format is the `exec` structure, which defines the layout of an executable file:


struct exec {
  unsigned long a_magic;    /* Magic number identifying the file type */
  unsigned a_text;          /* Length of text (code) section in bytes */
  unsigned a_data;          /* Length of initialized data section in bytes */
  unsigned a_bss;           /* Length of uninitialized data section (Block Started by Symbol) */
  unsigned a_syms;          /* Length of symbol table in bytes */
  unsigned a_entry;         /* Program entry point (start address) */
  unsigned a_trsize;        /* Length of text relocation info in bytes */
  unsigned a_drsize;        /* Length of data relocation info in bytes */
};


This structure appears at the beginning of every a.out executable and provides essential information about the size and layout of various sections within the file.

## Magic Numbers and File Types

The a.out format uses "magic numbers" to identify different types of executable files:


/* Code indicating object file or impure executable */
#define OMAGIC 0407

/* Code indicating pure executable */
#define NMAGIC 0410

/* Code indicating demand-paged executable */
#define ZMAGIC 0413


These magic numbers served distinct purposes:

- **OMAGIC (0407)**: The earliest and simplest format. Text and data segments are loaded contiguously in memory and both are writable.
- **NMAGIC (0410)**: Text segment is read-only and loaded at a fixed address, followed by the data segment.
- **ZMAGIC (0413)**: Demand-paged executable format with the text segment loaded at page boundaries to enable shared memory and demand paging.

The `N_BADMAG` macro checks if a file has a valid a.out magic number:


#define N_BADMAG(x)                    \
 (N_MAGIC(x) != OMAGIC && N_MAGIC(x) != NMAGIC        \
  && N_MAGIC(x) != ZMAGIC)


## Section Offsets

The header defines macros to calculate the file offset of each section:


/* Text section offset */
#define N_TXTOFF(x) \
 (N_MAGIC(x) == ZMAGIC ? _N_HDROFF((x)) + sizeof (struct exec) : sizeof (struct exec))

/* Data section offset */
#define N_DATOFF(x) (N_TXTOFF(x) + (x).a_text)

/* Text relocation info offset */
#define N_TRELOFF(x) (N_DATOFF(x) + (x).a_data)

/* Data relocation info offset */
#define N_DRELOFF(x) (N_TRELOFF(x) + (x).a_trsize)

/* Symbol table offset */
#define N_SYMOFF(x) (N_DRELOFF(x) + (x).a_drsize)

/* String table offset */
#define N_STROFF(x) (N_SYMOFF(x) + (x).a_syms)


These macros are critical for tools that need to read or write a.out files, as they define the precise layout of the file format.

## Memory Layout

After loading an executable, different sections have specific addresses in memory:


/* Address of text segment in memory after loading */
#define N_TXTADDR(x) 0

/* Calculate the end address of the text segment */
#define _N_TXTENDADDR(x) (N_TXTADDR(x)+(x).a_text)

/* Address of data segment in memory */
#define N_DATADDR(x) \
    (N_MAGIC(x)==OMAGIC? (_N_TXTENDADDR(x)) \
     : (_N_SEGMENT_ROUND (_N_TXTENDADDR(x))))

/* Address of BSS segment in memory */
#define N_BSSADDR(x) (N_DATADDR(x) + (x).a_data)


The memory layout varies depending on the magic number:
- For OMAGIC files, data starts immediately after text
- For NMAGIC and ZMAGIC files, data starts at a segment boundary after text

## Symbol Table Entries

A symbol table entry is defined by the `nlist` structure:


struct nlist {
  union {
    char *n_name;       /* Symbol name (in memory) */
    struct nlist *n_next; /* Used for linked lists */
    long n_strx;        /* String table index (in file) */
  } n_un;
  unsigned char n_type;  /* Type of symbol */
  char n_other;         /* Miscellaneous info */
  short n_desc;         /* Description field */
  unsigned long n_value; /* Value of symbol (usually address) */
};


### Symbol Types

Symbols are classified into different types:


#define N_UNDF 0    /* Undefined symbol */
#define N_ABS 2     /* Absolute symbol (doesn't change during linking) */
#define N_TEXT 4    /* Symbol in text section */
#define N_DATA 6    /* Symbol in data section */
#define N_BSS 8     /* Symbol in BSS section */
#define N_COMM 18   /* Common symbol (uninitialized data) */
#define N_FN 15     /* File name symbol */
#define N_INDR 0xa  /* Indirect reference to another symbol */


Additional flags can be combined with these types:


#define N_EXT 1     /* External symbol (global scope) */
#define N_TYPE 036  /* Mask for symbol type bits */
#define N_STAB 0340 /* Mask for debugging symbols */


## Relocation Information

The `relocation_info` structure defines how symbols are relocated during linking:


struct relocation_info {
  int r_address;                /* Address to be relocated (offset within segment) */
  unsigned int r_symbolnum:24;  /* Symbol number or segment */
  unsigned int r_pcrel:1;       /* Is this PC-relative? */
  unsigned int r_length:2;      /* Size of field to relocate (as power of 2) */
  unsigned int r_extern:1;      /* External symbol vs. segment relocation */
  unsigned int r_pad:4;         /* Unused padding bits */
};


This structure enables the linker to:
- Adjust references when combining object files
- Resolve external symbols
- Handle position-independent code through PC-relative addressing

## Historical Significance

The a.out format was the standard executable format in early UNIX systems, including BSD. While it has largely been replaced by more modern formats like ELF (Executable and Linkable Format), understanding a.out is valuable for:

1. Working with legacy systems
2. Studying the evolution of executable formats
3. Understanding fundamental concepts in linking and loading

Despite its simplicity, a.out established the fundamental principles of separating code (text), initialized data, and uninitialized data (BSS) that continue in modern executable formats.

