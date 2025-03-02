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

void PT_SetPTE(int pid, int VPN, int PFN, int valid, int protection, int present, int referenced) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (VPN * 4);

    physmem[entry] = PFN;
    physmem[entry + 1] = valid;
    physmem[entry + 2] = protection;
    physmem[entry + 3] = present;
}
int PT_PageTableInit(int pid, int pa) {
    char* physmem = Memsim_GetPhysMem();

    for (int i = 0; i < NUM_PAGES * 4; i += 4) {
        physmem[pa + i] = -1;
    }

    ptRegVals[pid].ptStartPA = pa;
    ptRegVals[pid].present = 1;

    // Move the print statement here so it gets printed even if PT_PageTableInit is called elsewhere
    printf("Put page table for PID %d into physical frame %d\n", pid, pa);
    
    return pa;
}


void PT_PageTableCreate(int pid, int pa) {
    if (!PT_PageTableExists(pid)) {
        PT_PageTableInit(pid, pa);
    }
}

int PT_PageTableExists(int pid) {
    return ptRegVals[pid].present;
}

int PT_GetRootPtrRegVal(int pid) {
    if (!PT_PageTableExists(pid)) {
        int pa = Memsim_FirstFreePFN();
        PT_PageTableCreate(pid, pa);
    }
    return ptRegVals[pid].ptStartPA;
}

int PT_Evict() {
    char* physmem = Memsim_GetPhysMem();
    FILE* swapFile = MMU_GetSwapFileHandle();
    int evictPage = pageToEvict * PAGE_SIZE;
    pageToEvict = (pageToEvict + 1) % NUM_PAGES;
    fwrite(&physmem[evictPage], PAGE_SIZE, 1, swapFile);
    memset(&physmem[evictPage], 0, PAGE_SIZE);
    return evictPage;
}

int PT_VPNtoPA(int pid, int VPN) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (VPN * 4);

    if (physmem[entry + 1] == 1) {
        return physmem[entry] * PAGE_SIZE;
    }
    return -1;
}

int PT_PIDHasWritePerm(int pid, int VPN) {
    char* physmem = Memsim_GetPhysMem();
    int pt_base = PT_GetRootPtrRegVal(pid);
    int entry = pt_base + (VPN * 4);

    if (physmem[entry + 1] == 1 && physmem[entry + 2] == 1) {
        return TRUE;
    }
    return FALSE;
}

void PT_SetSwapped(int frame, int swapSlot) {
    // Implementation for marking the page as swapped
    printf("Frame %d is swapped to swap slot %d\n", frame, swapSlot);
}


void PT_Init() {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        ptRegVals[i].ptStartPA = -1;
        ptRegVals[i].present = 0;
    }
}