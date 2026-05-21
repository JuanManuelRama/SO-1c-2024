# Operating System Simulator 🐧

This repository contains the implementation of a distributed operating system simulator, developed for the Operating Systems course (1C2024). The system models the interaction between core OS components, managing processes, memory paging, CPU scheduling, and file systems.

## System Architecture
The simulator is divided into four standalone, communicating modules:

### Kernel
The Kernel module manages the lifecycle and scheduling of processes through an interactive console.
* Implements a 5-state process lifecycle: NEW, READY, EXEC, BLOCKED, and EXIT.
* Utilizes Process Control Blocks (PCB) to track execution context, including the process PID, Program Counter, and CPU registers.
* Features a Short-Term Scheduler supporting FIFO, Round Robin (RR), and Virtual Round Robin (VRR) algorithms.
* Manages shared system resources, handling WAIT and SIGNAL synchronization requests.

### CPU
The CPU module interprets and executes instructions from the processes dispatched by the Kernel.
* Simulates a simplified instruction cycle consisting of Fetch, Decode, Execute, and Check Interrupt phases.
* Simulates a Memory Management Unit (MMU) to translate logical addresses to physical addresses using a paging scheme.
* Includes a Translation Lookaside Buffer (TLB) to optimize memory translations, utilizing either FIFO or LRU replacement algorithms.
* Processes a custom instruction set, including arithmetic operations, memory I/O, and system calls.

### Memory
The Memory module manages both the instruction sets for processes and the simulated physical RAM.
* Implements a simple paging scheme utilizing a contiguous user space and page tables.
* Handles dynamic memory requests, allowing processes to resize by expanding or reducing their allocated frames.
* Introduces artificial response delays, configured via parameter files, to simulate real hardware latency.

### I/O Interfaces
This module handles external peripheral simulation, processing requests sequentially. It supports four types of interfaces:
* **Generic:** Simulates basic devices by waiting for a specified number of work units before completing a request.
* **STDIN:** Reads text input from the keyboard and stores it directly into memory.
* **STDOUT:** Reads data from memory and outputs it to the console screen.
* **DialFS:** A custom file system implementing contiguous block allocation. It manages block files, a free-space bitmap, and metadata files. It also supports dynamic file truncation and file system compaction to resolve fragmentation.

## Deployment & Configuration
This project relies on the [so-commons-library](https://github.com/sisoputnfrba/so-commons-library) for core utilities, logging, and configuration management.

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

The system is designed as a distributed platform, meaning the distinct processes can be executed across multiple different computers. Connections between the Kernel, CPU, Memory, and I/O modules are established using IP addresses and ports defined in specific configuration files for each component. Each module is compiled through a makefile, by using the make command on the module folder
