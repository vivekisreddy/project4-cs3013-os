// Starting code version 1.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include "mmu.h"
#include "pagetable.h"
#include "memsim.h"

int pageToEvict = 1; //if using round robin eviction, this can be used to keep track of the next page to evict

// Page table root pointer register values (one stored per process, would be "swapped in" to MMU with process, when scheduled)
typedef struct{
	int ptStartPA;
	int present;
} ptRegister;

// Page table root pointer register values 
// One stored for each process, swapped in to MMU when process is scheduled to run)
ptRegister ptRegVals[NUM_PROCESSES]; 

/*
 * Public Interface:
 */

/*
 * Sets the Page Table Entry (PTE) for the given process and VPN.
 * The PTE contains the PFN, valid bit, protection bit, present bit, and referenced bit.
 */
void PT_SetPTE(int pid, int VPN, int PFN, int valid, int protection, int present, int referenced) {
	char* physmem = Memsim_GetPhysMem();
	assert(PT_PageTableExists(pid)); // page table should exist if this is being called

	int pt_start = PT_GetRootPtrRegVal(pid); // Get the physical address where the page table starts

    int entry_offset = VPN * 3; // Each PTE is 3 bytes long (1st byte: flags, 2nd: VPN, 3rd: PFN)
    int entry_addr = pt_start + entry_offset;

    // Construct the first byte with flags: Valid (bit 0), Protection (bit 1), Present (bit 2), Referenced (bit 3)
    char flags = (valid & 0x1) | ((protection & 0x1) << 1) | ((present & 0x1) << 2) | ((referenced & 0x1) << 3);

    // Write the PTE to memory
    physmem[entry_addr] = flags;
    physmem[entry_addr + 1] = (char)VPN;
    physmem[entry_addr + 2] = (char)PFN;
}

/* 
 * Initializes the page table for the process, and sets the starting physical address for the page table.
 * 
 * After initialization, get the next free page in physical memory.
 * If there are no free pages, evict a page to get a newly freed physical address.
 * Finally, return the physical address of the next free page.
 */
int PT_PageTableInit(int pid, int pa){
	char* physmem = Memsim_GetPhysMem();

	// Check if there is a free page available; if not, evict one
    if (pa == -1) {
        pa = PT_Evict();  // Evict a page and use the newly freed space
    }

    // Zero out the page we are about to use for the page table
    memset(&physmem[pa], 0, PAGE_SIZE);

    // Set the page table's root pointer register value
    PT_SetPTE(pid, 0, pa, 1, 0, 1, 0);  // Initialize the first entry of the page table

    // Return the PA of the next free page
    return pa + PAGE_SIZE;  // Assuming pages are contiguous
 }

/* 
 * Check the ptRegVars to see if there is a valid starting PA for the given PID's page table.
 * Returns true (non-zero) or false (zero).
 */
 int PT_PageTableExists(int pid){
 	if (pid < 0 || pid >= NUM_PROCESSES) {
        return 0; // Invalid process ID
    }
    return PT_GetRootPtrRegVal(pid) != -1; // Assuming -1 means no page table assigned 
 }

/* 
 * Returns the starting physical address of the page table for the given PID.
 * If the page table does not exist, returns -1.
 * If the page table is not in memory, first swaps it in to physical memory.
 * Finally, returns the starting physical address of the page table.
 */
int PT_GetRootPtrRegVal(int pid){
	// Check if the page table exists
    if (!PT_PageTableExists(pid)) {
        return -1; // Page table does not exist
    }
    // If the page table is not in memory, evict a page to bring it into memory
    int rootPA = PT_PageTableInit(pid, PT_Evict());

    return rootPA;
}

/*
 * Evicts the next page. 
 * Updates the corresponding information in the page table, returns the PA of the evicted page.
 * 
 * The supplied input and output used in autotest.sh *RR tests, uses the round-robin algorithm. 
 * You may also implement the simple and powerful Least Recently Used (LRU) policy, 
 * or another fair algorithm.
 */
int PT_Evict() {
    static int nextEvictIndex = 0; // Keeps track of the next page to evict (round-robin)
    char* physmem = Memsim_GetPhysMem();
    FILE* swapFile = MMU_GetSwapFileHandle();

    if (!physmem || !swapFile) {
        return -1; // Error handling: Unable to access physical memory or swap file
    }

    // Identify the page to evict
    int evictPage = nextEvictIndex;
    nextEvictIndex = (nextEvictIndex + 1) % NUM_PAGES; // Move to the next page in a circular manner

    return PAGE_START(evictPage); // Return the physical address of the evicted page
}

/*
 * Searches through the process's page table. If an entry is found containing the specified VPN, 
 * return the address of the start of the corresponding physical page frame in physical memory. 
 *
 * If the physical page is not present, first swaps in the phyical page from the physical disk,
 * and returns the physical address.
 * 
 * Otherwise, returns -1.
 */
int PT_VPNtoPA(int pid, int VPN) {
    if (!PT_PageTableExists(pid)) {
        return -1;  // The process does not have a page table
    }

    int rootAddress = PT_GetRootPtrRegVal(pid);  // Get page table base address
    int PA = *(int*)(Memsim_GetPhysMem() + rootAddress + (VPN * sizeof(int)));  // Get PA from page table

    if (PA == -1) {
        return -1;  // Page not found
    }

    return PA;  // Return the physical address
}

/*
 * Finds the page table entry corresponding to the VPN, and checks
 * to see if the protection bit is set to 1 (readable and writable).
 * If it is 1, it returns TRUE, and FALSE if it is not found or is 0.
 */
int PT_PIDHasWritePerm(int pid, int VPN) {
    if (!PT_PageTableExists(pid)) {
        return FALSE;  // Process does not have a page table
    }

    int rootAddress = PT_GetRootPtrRegVal(pid);  // Get page table base address
    int entryIndex = rootAddress + (VPN * sizeof(int) * 4); // Assuming each entry has multiple fields
    int protectionBit = *(int*)(Memsim_GetPhysMem() + entryIndex + sizeof(int)); // Offset for protection bit

    return (protectionBit == 1) ? TRUE : FALSE; // Return TRUE if writable, FALSE otherwise
}

/* 
 * Initialize the register values for each page table location (per process).
 * For example, -1 for the starting physical address of the page table, and FALSE for present.
 */
void PT_Init() {
	for (int pid = 0; pid < NUM_PROCESSES; pid++) {
        // Initialize the page table for each process
        PT_PageTableInit(pid, -1); // Set the initial physical address to -1 (not allocated)
    }
}