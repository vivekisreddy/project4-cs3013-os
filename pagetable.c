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

typedef struct {
    int ptStartPA;
    int present;
} ptRegister;

ptRegister ptRegVals[NUM_PROCESSES];  // Stores page table root for each process

// Function to set a page table entry
void PT_SetPTE(int pid, int VPN, int PFN, int valid, int protection, int present, int referenced) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (VPN * 5);  // 5 bytes per entry now to store swap info

    physmem[entry] = PFN;
    physmem[entry + 1] = valid;
    physmem[entry + 2] = protection;
    physmem[entry + 3] = present;
    physmem[entry + 4] = referenced;  // Use this for swap_offset if needed
}

PageTableEntry* PT_GetPTE(int pid, int vpn) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (vpn * 5);

    static PageTableEntry pte;
    pte.PFN = physmem[entry];
    pte.valid = physmem[entry + 1];
    pte.protection = physmem[entry + 2];
    pte.present = physmem[entry + 3];
    pte.referenced = physmem[entry + 4];
    pte.rw_bit = pte.protection;  // Assuming rw_bit is the same as protection flag

    if (!pte.valid) return NULL; // If entry is invalid, return NULL

    return &pte;
}

// Initialize a page table for a process
int PT_PageTableInit(int pid, int pa) {
    char* physmem = Memsim_GetPhysMem();

    for (int i = 0; i < NUM_PAGES * 5; i += 5) {
        physmem[pa + i] = -1;  // Mark all VPNs as invalid
    }

    ptRegVals[pid].ptStartPA = pa;
    ptRegVals[pid].present = 1;

    printf("Put page table for PID %d into physical frame %d\n", pid, pa);
    
    return pa;
}

// Create page table if it does not exist
void PT_PageTableCreate(int pid, int pa) {
    if (!PT_PageTableExists(pid)) {
        PT_PageTableInit(pid, pa);
    }
	
}

// Check if page table exists
int PT_PageTableExists(int pid) {
    return ptRegVals[pid].present;
}

// Get root pointer register value (i.e., page table location)
int PT_GetRootPtrRegVal(int pid) {
    if (!PT_PageTableExists(pid)) {
        int pa = Memsim_FirstFreePFN();
        PT_PageTableCreate(pid, pa);
    }
    return ptRegVals[pid].ptStartPA;
}

// Evict a page from memory to swap file
int PT_Evict() {
    char* physmem = Memsim_GetPhysMem();
    FILE* swapFile = MMU_GetSwapFileHandle();
    
    int evictPage = pageToEvict * PAGE_SIZE;
    pageToEvict = (pageToEvict + 1) % NUM_PAGES;  // Round-Robin eviction

    // Write the page to swap space
    fseek(swapFile, evictPage, SEEK_SET);
    fwrite(&physmem[evictPage], PAGE_SIZE, 1, swapFile);
    
    // Mark page as swapped and clear memory
    memset(&physmem[evictPage], 0, PAGE_SIZE);

    return evictPage;  // Return swap offset
}

int PT_VPNtoPA(int pid, int VPN) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (VPN * 5);

    if (physmem[entry + 1] == 1) { // Page is in memory
        return (physmem[entry] * PAGE_SIZE);  
    }

    int swap_offset = physmem[entry + 4];
    if (swap_offset >= 0) {
        int newFrame = Memsim_SwapIn(pid, VPN, swap_offset);
        if (newFrame == -1) return -1;
        return newFrame * PAGE_SIZE;
    }

    return -1;  // Page not found
}



// Check if process has write permissions to a page
int PT_PIDHasWritePerm(int pid, int VPN) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (VPN * 5);

    if (physmem[entry + 1] == 1 && physmem[entry + 2] == 1) {
        return TRUE;
    }
    return FALSE;
}

int PT_GetWritePerm(int pid, int vpn) {
    PageTableEntry* pte = PT_GetPTE(pid, vpn);
    if (pte == NULL) return 0; // If no mapping exists, assume no write permission
    return pte->rw_bit; // Return read/write permission bit
}

void PT_UpdateWritePerm(int pid, int vpn, int new_perm) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (vpn * 5); // Offset for 5-byte page table entry

    physmem[entry + 2] = new_perm; // Update write permission
}



// Initialize all process page tables
void PT_Init() {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        ptRegVals[i].ptStartPA = -1;
        ptRegVals[i].present = 0;
    }
}
