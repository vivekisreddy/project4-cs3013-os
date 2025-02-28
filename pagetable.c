// Starting code version 1.0 

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

int pageToEvict = 1;

typedef struct{
	int ptStartPA;
	int present;
} ptRegister;

// Page table root pointer register values 
// One stored for each process, swapped in with process)
ptRegister ptRegVals[NUM_PROCESSES]; 

/*
 * Public Interface:
 */
void PT_SetPTE(int pid, int VPN, int PFN, int valid, int protection, int present, int referenced) {
	char* physmem = Memsim_GetPhysMem();
	//todo
}

/* 
 * Set all PTE valid bits to zero (invalid)
 * Returns a new page for the map instruction
 */
int PT_PageTableInit(int pid, int pa){
	char* physmem = Memsim_GetPhysMem();
	// todo
	return 0;
 }

 void PT_PageTableCreate(int pid, int pa){
	//todo
 }

 int PT_PageTableExists(int pid){
	//todo
 }

/* Gets the location of the start of the page table. If it is on disk, brings it into memory. */
int PT_GetRootPtrRegVal(int pid){
	// todo
	return 0;
}

/*
 * Evicts the next page. 
 * Updates the corresponding information in the page table, returns the location that was evicted.
 * 
 * The supplied input and output used in autotest.sh *RR tests, uses the round-robin algorithm. 
 * You may also implement the simple and powerful Least Recently Used (LRU) policy, 
 * or another fair algorithm.
 */
int PT_Evict() {
	char* physmem = Memsim_GetPhysMem();
	FILE* swapFile = MMU_GetSwapFileHandle();
	//todo
	return 0;
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
	//todo
	return 0;
}

/* Initialize the register values for each page table location (per process). */
void PT_Init() {
	//todo
}