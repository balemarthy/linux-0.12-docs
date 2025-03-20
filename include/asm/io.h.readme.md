# **README - I/O Port Macros in Linux 0.12**

## **Overview**
These macros facilitate direct interaction with **I/O ports** at the hardware level using **inline assembly**. They are used to send and receive data between the CPU and peripheral devices such as **keyboards, disks, and network interfaces**.

Each macro utilizes x86 **IN** and **OUT** instructions to read from or write to **hardware ports**. These operations are crucial for **low-level system programming** and **driver development**.

---

## **1. `outb(value, port)` - Output Byte to I/O Port**

#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))


### **Explanation**
- **Purpose:** Sends a **byte (`value`)** to a specified **I/O port (`port`)**.
- **Inline Assembly Breakdown:**
  1. `"outb %%al,%%dx"` → Outputs **AL register** value to **DX register** (I/O port).
  2. `"a" (value)` → Moves `value` into register **AL** (low byte of EAX), required for **OUT** instruction.
  3. `"d" (port)` → Moves `port` number into register **DX**, specifying the target port.
  4. **No return value**, just **sends data** to the hardware.

---

## **2. `inb(port)` - Input Byte from I/O Port**

#define inb(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
_v; \
})


### **Explanation**
- **Purpose:** Reads a **byte** from a specified **I/O port** into a variable.
- **Inline Assembly Breakdown:**
  1. `"inb %%dx,%%al"` → Reads from **DX (port number)** into **AL** register.
  2. `"=a" (_v)` → Stores **AL value** (byte received) into `_v`.
  3. `"d" (port)` → Moves `port` into **DX**, specifying the I/O port.
  4. **Returns** `_v`, the received byte.
  5. `"volatile"` ensures the compiler does not optimize away this essential hardware instruction.

---

## **3. `outb_p(value, port)` - Output Byte with Delay**

#define outb_p(value,port) \
__asm__ ("outb %%al,%%dx\n" \
        "\tjmp 1f\n" \
        "1:\tjmp 1f\n" \
        "1:"::"a" (value),"d" (port))


### **Explanation**
- **Purpose:** Sends a byte to an **I/O port**, but adds a slight delay afterward.
- **Why a delay?** Some hardware requires **time to process** I/O operations before accepting new data.
- **Inline Assembly Breakdown:**
  1. `"outb %%al,%%dx"` → Writes **AL register** value to **DX (I/O port)**.
  2. `"jmp 1f"` (twice) → Executes **two jumps**, introducing a **tiny delay**.
  3. `"a" (value)` → Moves `value` into **AL**.
  4. `"d" (port)` → Moves `port` into **DX**.

---

## **4. `inb_p(port)` - Input Byte with Delay**

#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
    "\tjmp 1f\n" \
    "1:\tjmp 1f\n" \
    "1":"=a" (_v):"d" (port)); \
_v; \
})


### **Explanation**
- **Purpose:** Reads a byte from an **I/O port**, but introduces a **delay** after reading.
- **Why a delay?** Some hardware devices may require a short pause after reading before they are ready for the next instruction.
- **Inline Assembly Breakdown:**
  1. `"inb %%dx,%%al"` → Reads a byte from **DX (port)** into **AL** register.
  2. `"jmp 1f"` (twice) → Introduces a **small delay**.
  3. `"=a" (_v)` → Stores the **received value** into `_v`.
  4. `"d" (port)` → Moves the **port number** into `DX`.
  5. **Returns** `_v`, the received byte.

---

## **Summary of I/O Macros**
| **Macro** | **Operation** | **Delay?** | **Purpose** |
|-----------|--------------|------------|-------------|
| `outb(value, port)` | Write byte to port | ❌ No | Sends data to an I/O port |
| `inb(port)` | Read byte from port | ❌ No | Receives data from an I/O port |
| `outb_p(value, port)` | Write byte to port | ✅ Yes | Sends data with delay |
| `inb_p(port)` | Read byte from port | ✅ Yes | Receives data with delay |

---

## **Why Are These Macros Important?**
- **Direct hardware communication:** These macros allow the kernel to **control devices** like disk controllers, keyboards, and network interfaces.
- **Performance optimization:** Using inline assembly ensures **minimal overhead**.
- **Hardware compatibility:** Some devices **require delays** (`outb_p`, `inb_p`) to function correctly.


