# OS

Working on HW for the course "Operating System" (https://stepik.org/course/1780/info)

### Mapping logical address to the physical address for x86 arch
- #### See [mapping_vAddr_to_phAddr_x86](https://github.com/Montura/OS/blob/master/src/mapping_vAddr_to_phAddr_x86.cpp)

#### Paging: Logical address in x86 (Long Mode)
|64-bit address| | | | | | |
|:---:| :-----------: |:---:|:---:|:---:|:---:|:---:|
|Bits|63 ... 48|47 ... 39|38 ... 30|29 ... 21|20 ... 12|11 ... 0|
|Values| [63:48] = 47 bit |    PLM4    |  DirectoryPtr   | Directory |  Table  |    Offset    |

### Read ELF header:
- #### See [read_elf](https://github.com/Montura/OS/blob/master/src/read_elf.cpp)
1) To get the address of main entry point
2) To get the size of memory required to load the program

### Thread experiments
1. [Priority boost (Windows)](https://github.com/Montura/OS/blob/master/src/threads/priority_boost_win.cpp)
2. [Robin round algorithm](https://github.com/Montura/OS/blob/master/src/threads/robin_round.cpp)
3. [Thread synchronization](https://github.com/Montura/OS/blob/master/src/threads/thread_synchronization.cpp)
    1. Alternation lock
    2. IntentionFlags
    3. Peterson Algorithm
    4. Peterson Greedy
    5. OptimizedPeterson
4. [Atomic increment and CAS for amomic RWM register uisng LL (load-linked) and SC (store-conditional)](https://github.com/Montura/OS/blob/master/src/threads/rmw_register.cpp)
5. [Mutual exculison with Read-Modify-Write register nad Ticket lock](https://github.com/Montura/OS/blob/master/src/threads/rwm_locks.cpp)
6. [Readers|Writers: Read-Write lock ](https://github.com/Montura/OS/blob/master/src/threads/read_write_lock.cpp)
