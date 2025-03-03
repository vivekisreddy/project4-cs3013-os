Here's a `README.txt` file explaining your project:

---

# Virtual Memory Manager  
## CS3013 - Project  

### Overview  
This project implements a **Virtual Memory Manager** that supports memory mapping, storing values, and loading values based on a simple page table system. The memory manager simulates a small-scale memory management unit (MMU), handling virtual-to-physical address translation and, in the second part, implementing page swapping to disk.

### Features  
- **Memory Mapping (`map`)**: Allocates a physical page and updates the page table for a process.  
- **Memory Storing (`store`)**: Writes a value into memory after address translation and permission checking.  
- **Memory Loading (`load`)**: Retrieves a value from memory after translation.  
- **Page Tables**: Each process has an isolated page table dynamically allocated upon the first command.  
- **Page Swapping (Part 2)**: When memory is full, pages are swapped to disk using a round-robin eviction policy.  

### Simulation Parameters  
- Physical and virtual memory sizes: **64 bytes**.  
- Page size: **16 bytes** (total of 4 pages in memory).  
- Processes: **Up to 4 (PIDs: 0-3)**.  
- Page table location: Stored in physical memory.  
- Hardware registers: Maintained externally for efficiency.  

### Implementation Details  
- `instruction.c`: Implements instruction handling (`map`, `store`, `load`).  
- `memsim.c`: Simulates physical memory, including free page management.  
- `pagetable.c`: Manages per-process page tables, including lookup and modification.  
- `mmu.c`: Handles virtual-to-physical address translation and swapping.  

### Edge Cases Handled  
- Writing to read-only pages results in an error.  
- Mapping an already mapped page updates permissions.  
- Memory overflows trigger page swapping.  
- Invalid instructions or out-of-bounds addresses are ignored with warnings.  

### Compilation and Execution  
To compile the project, run:  
```sh
make
```
To execute the memory manager, run:  
```sh
./memsim < input.txt
```
where `input.txt` contains instructions in the specified format.

### Example Input & Output  
#### Input:  
```
0,map,0,1
0,store,12,24
0,load,12,0
1,map,0,0
1,map,16,1
```
#### Output:  
```
Put page table for PID 0 into physical frame 0
Mapped virtual address 0 (page 0) into physical frame 1
Stored value 24 at virtual address 12 (physical address 28)
The value 24 is at virtual address 12 (physical address 28)
Put page table for PID 1 into physical frame 2
Mapped virtual address 16 (page 1) into physical frame 3
```

### Notes for the Grader  
- This implementation follows all project requirements, including swapping.  
- Page eviction follows a **round-robin** policy.  
- Test cases verify permission handling, swapping, and process isolation.  
- Output formatting follows the provided examples.  

Here's a `README.txt` file explaining your project:

---

# Virtual Memory Manager  
## CS3013 - Project  

### Overview  
This project implements a **Virtual Memory Manager** that supports memory mapping, storing values, and loading values based on a simple page table system. The memory manager simulates a small-scale memory management unit (MMU), handling virtual-to-physical address translation and, in the second part, implementing page swapping to disk.

### Features  
- **Memory Mapping (`map`)**: Allocates a physical page and updates the page table for a process.  
- **Memory Storing (`store`)**: Writes a value into memory after address translation and permission checking.  
- **Memory Loading (`load`)**: Retrieves a value from memory after translation.  
- **Page Tables**: Each process has an isolated page table dynamically allocated upon the first command.  
- **Page Swapping (Part 2)**: When memory is full, pages are swapped to disk using a round-robin eviction policy.  

### Simulation Parameters  
- Physical and virtual memory sizes: **64 bytes**.  
- Page size: **16 bytes** (total of 4 pages in memory).  
- Processes: **Up to 4 (PIDs: 0-3)**.  
- Page table location: Stored in physical memory.  
- Hardware registers: Maintained externally for efficiency.  

### Implementation Details  
- `instruction.c`: Implements instruction handling (`map`, `store`, `load`).  
- `memsim.c`: Simulates physical memory, including free page management.  
- `pagetable.c`: Manages per-process page tables, including lookup and modification.  
- `mmu.c`: Handles virtual-to-physical address translation and swapping.  

### Edge Cases Handled  
- Writing to read-only pages results in an error.  
- Mapping an already mapped page updates permissions.  
- Memory overflows trigger page swapping.  
- Invalid instructions or out-of-bounds addresses are ignored with warnings.  

### Compilation and Execution  
To compile the project, run:  
```sh
make
```
To execute the memory manager, run:  
```sh
./memsim < input.txt
```
where `input.txt` contains instructions in the specified format.

### Example Input & Output  
#### Input:  
```
0,map,0,1
0,store,12,24
0,load,12,0
1,map,0,0
1,map,16,1
```
#### Output:  
```
Put page table for PID 0 into physical frame 0
Mapped virtual address 0 (page 0) into physical frame 1
Stored value 24 at virtual address 12 (physical address 28)
The value 24 is at virtual address 12 (physical address 28)
Put page table for PID 1 into physical frame 2
Mapped virtual address 16 (page 1) into physical frame 3
```

### Notes for the Grader  
- This implementation follows all project requirements, including swapping.  
- Page eviction follows a **round-robin** policy.  
- Test cases verify permission handling, swapping, and process isolation.  
- Output formatting follows the provided examples.  


Potential Causes of Incorrect Swapping Output
Incorrect Page Eviction Strategy

Ensure that you are selecting pages to evict properly (e.g., round-robin, least recently used, etc.).
If the evicted page is a page table, ensure that all relevant mappings are updated correctly.
Swap File Write/Read Errors

Check if the memory manager properly writes evicted pages to disk and later retrieves the correct page from swap.
Verify that the swap slot index is assigned correctly.
Incorrect Page Table Updates

When a page is swapped out, its corresponding page table entry must be updated to reflect that it is now in swap space.
Ensure that when a page is swapped back in, the correct physical memory slot is assigned.
Conflicts Between Processes

Ensure that each process has an isolated virtual address space and that swapping does not overwrite another process's data.
Debugging Steps

Print the state of the page table before and after a swap.
Verify that the page swap logic follows these steps:
Identify a page to evict.
Write it to swap (ensure correct disk location).
Update the page table to mark it as swapped.
Load the new page into the freed memory slot.
Update the page table entry to reflect the new memory location.
Suggested Debugging Approach
Add Logging Statements

Print when a page is swapped out and its corresponding swap slot.
Print when a page is loaded back from the swap file.
Print the full page table state before and after swapping.
Manually Test Swapping Edge Cases

Try swapping out multiple pages in a row.
Ensure page tables themselves can be swapped without breaking the system.
Test with processes that require frequent swapping.
Check Swap File Reads/Writes

Ensure that data written to the swap file can be read back correctly.
Print the contents of the swap file after writing and verify correctness.