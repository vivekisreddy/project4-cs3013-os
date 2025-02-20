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

	//todo 
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

	// todo 
	// zero out the page we are about to use for the page table 

	// set the page table's root pointer register value

	// return the PA of the next free page
	
	// If there were no free pages,
	// Evict one and use the new space
	
	return pa;
 }

/* 
 * Check the ptRegVars to see if there is a valid starting PA for the given PID's page table.
 * Returns true (non-zero) or false (zero).
 */
 int PT_PageTableExists(int pid){
 	return 0; //todo 
 }

/* 
 * Returns the starting physical address of the page table for the given PID.
 * If the page table does not exist, returns -1.
 * If the page table is not in memory, first swaps it in to physical memory.
 * Finally, returns the starting physical address of the page table.
 */
int PT_GetRootPtrRegVal(int pid){
	//todo
	// If the page table does not exist, return -1

	// If the page table is not in memory, swap it in

	// Return the starting physical address of the page table
	return -1; 
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

	//todo

	return 0; //todo
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
int PT_VPNtoPA(int pid, int VPN){
	char *physmem = Memsim_GetPhysMem();

	//todo 

	return -1;
}

/*
 * Finds the page table entry corresponding to the VPN, and checks
 * to see if the protection bit is set to 1 (readable and writable).
 * If it is 1, it returns TRUE, and FALSE if it is not found or is 0.
 */
int PT_PIDHasWritePerm(int pid, int VPN){
	char* physmem = Memsim_GetPhysMem();

	return FALSE; //todo
}

/* 
 * Initialize the register values for each page table location (per process).
 * For example, -1 for the starting physical address of the page table, and FALSE for present.
 */
void PT_Init() {

	return; //todo
}