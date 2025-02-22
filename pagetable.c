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

// Define the structure for a Page Table Entry (PTE)
typedef struct {
    int PFN;        // Physical Frame Number
    int valid;      // Valid bit (1 if the entry is valid, 0 otherwise)
    int protection; // Protection bit (e.g., 1 for read-write, 0 for read-only)
    int present;    // Present bit (1 if the page is in memory, 0 if swapped out)
    int referenced; // Referenced bit (1 if the page has been accessed)
} PTE;


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
    assert(PT_PageTableExists(pid)); // Ensure the page table exists

    // Get the physical address where the page table starts for this process
    int ptStartPA = PT_GetRootPtrRegVal(pid);
    if (ptStartPA == -1) {
        printf("Error: Page Table does not exist for PID %d\n", pid);
        return;
    }

    // Calculate the offset for the PTE in the page table based on the VPN
    int ptEntryOffset = VPN * sizeof(PTE); // Assuming PTE size is sizeof(PTE)

    // Access the page table entry for the given VPN
    PTE* pte = (PTE*)(physmem + ptStartPA + ptEntryOffset);

    // Set the PTE fields
    pte->PFN = PFN;
    pte->valid = valid;
    pte->protection = protection;
    pte->present = present;
    pte->referenced = referenced;
}

/* 
 * Initializes the page table for the process, and sets the starting physical address for the page table.
 * 
 * After initialization, get the next free page in physical memory.
 * If there are no free pages, evict a page to get a newly freed physical address.
 * Finally, return the physical address of the next free page.
 */
int PT_PageTableInit(int pid, int pa) {
    char* physmem = Memsim_GetPhysMem();

    // Zero out the page table space for the process
    memset(physmem + pa, 0, PAGE_SIZE);

    // Set the starting physical address of the page table in the page table register
    ptRegVals[pid].ptStartPA = pa;
    ptRegVals[pid].present = 1;

    return pa;
}


/* 
 * Check the ptRegVars to see if there is a valid starting PA for the given PID's page table.
 * Returns true (non-zero) or false (zero).
 */
int PT_PageTableExists(int pid) {
    return ptRegVals[pid].present;
}


/* 
 * Returns the starting physical address of the page table for the given PID.
 * If the page table does not exist, returns -1.
 * If the page table is not in memory, first swaps it in to physical memory.
 * Finally, returns the starting physical address of the page table.
 */
int PT_GetRootPtrRegVal(int pid) {
    if (!PT_PageTableExists(pid)) {
        printf("Error: Page table does not exist for PID %d\n", pid);
        return -1;
    }

    // Check if the page table is in memory, otherwise swap it in
    if (!ptRegVals[pid].present) {
        PT_SwapIn(pid);
    }

    return ptRegVals[pid].ptStartPA;
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
    char* physmem = Memsim_GetPhysMem();
    FILE* swapFile = MMU_GetSwapFileHandle();

    // Select the page to evict based on the eviction policy (e.g., round-robin)
    int evictPage = pageToEvict++;
    if (pageToEvict >= NUM_PAGES) {
        pageToEvict = 0; // Wrap around
    }

    // Evict the page and return its physical address
    // For simplicity, just use the evictPage as the PFN
    return evictPage;
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
    char *physmem = Memsim_GetPhysMem();

    // Get the starting physical address of the page table for the given process
    int ptStartPA = PT_GetRootPtrRegVal(pid);
    if (ptStartPA == -1) {
        printf("Error: Page table not found for PID %d\n", pid);
        return -1;
    }

    // Calculate the PTE offset and retrieve the PTE
    int ptEntryOffset = VPN * sizeof(PTE);
    PTE* pte = (PTE*)(physmem + ptStartPA + ptEntryOffset);

    if (!pte->present) {
        // If the page is not present, swap it in
        PT_SwapIn(pid, VPN);
    }

    return pte->PFN * PAGE_SIZE + (VPN % PAGE_SIZE);  // Convert PFN to physical address
}


/*
 * Finds the page table entry corresponding to the VPN, and checks
 * to see if the protection bit is set to 1 (readable and writable).
 * If it is 1, it returns TRUE, and FALSE if it is not found or is 0.
 */
int PT_PIDHasWritePerm(int pid, int VPN) {
    char* physmem = Memsim_GetPhysMem();

    // Get the starting physical address of the page table for the given process
    int ptStartPA = PT_GetRootPtrRegVal(pid);
    if (ptStartPA == -1) {
        printf("Error: Page table not found for PID %d\n", pid);
        return FALSE;
    }

    // Calculate the PTE offset and retrieve the PTE
    int ptEntryOffset = VPN * sizeof(PTE);
    PTE* pte = (PTE*)(physmem + ptStartPA + ptEntryOffset);

    return pte->protection == 1;  // Check for write permission
}

/* 
 * Initialize the register values for each page table location (per process).
 * For example, -1 for the starting physical address of the page table, and FALSE for present.
 */
void PT_Init() {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        ptRegVals[i].ptStartPA = -1;  // Set initial page table address to -1 (invalid)
        ptRegVals[i].present = 0;  // Mark the page table as not present initially
    }
}
