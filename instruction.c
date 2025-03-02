// Starting code version v1.0

#include <stdio.h>
#include <stdint.h>

#include "memsim.h"
#include "pagetable.h"
#include "mmu.h"

/*
 * Searches the memory for a free page, and assigns it to the process's virtual address. If value is
 * 0, the page has read only permissions, and if it is 1, it has read/write permissions.
 * If the process does not already have a page table, one is assigned to it. If there are no more empty
 * pages, the mapping fails.
 */

int Instruction_Map(int pid, int va, int value_in) {
    int pa;
    char* physmem = Memsim_GetPhysMem();

    if (value_in != 0 && value_in != 1) {
        printf("Invalid value for map instruction. Value must be 0 or 1.\n");
        return 1;
    }
    if ((pa = PT_VPNtoPA(pid, VPN(va))) != -1) {
        printf("Error: Virtual page already mapped into physical frame %d.\n", PFN(pa));
        return 1;
    }

    // Ensure the first page table is in physical frame 0
    if (!PT_PageTableExists(pid)) {
        int pt_frame = (pid == 0) ? 0 : Memsim_FirstFreePFN();
        if (pt_frame == -1) {
            printf("Error: No available memory for page table.\n");
            return 1;
        }
        PT_PageTableCreate(pid, pt_frame);
        
        // Debug: Confirm page table creation
        printf("Put page table for PID %d into physical frame %d\n", pid, pt_frame);
    }

    // Allocate a frame for the virtual page
    pa = Memsim_FirstFreePFN();
    if (pa == -1) {
        pa = PT_Evict();
        if (pa == -1) {
            printf("Error: No available memory.\n");
            return 1;
        }
    }

    // Set the page table entry
    PT_SetPTE(pid, VPN(va), PFN(pa), 1, value_in, 1, 0);
    printf("Mapped virtual address %d (page %d) into physical frame %d.\n", va, VPN(va), PFN(pa));
    return 0;
}


int Instruction_Store(int pid, int va, int value_in) {
    int pa = MMU_TranslateAddress(pid, VPN(va), PAGE_OFFSET(va));
    if (pa == -1) {
        printf("Error: Virtual address %d is not mapped.\n", va);
        return 1;
    }
    if (!PT_PIDHasWritePerm(pid, VPN(va))) {
        printf("Error: Virtual address %d does not have write permissions.\n", va);
        return 1;
    }
    Memsim_Store(pa, value_in);
    printf("Stored value %u at virtual address %d (physical address %d)\n", value_in, va, pa);
    return 0;
}

int Instruction_Load(int pid, int va) {
    int pa = MMU_TranslateAddress(pid, VPN(va), PAGE_OFFSET(va));
    
    // Ensure address is valid
    if (pa == -1) {
        printf("Error: The virtual address %d is not valid.\n", va);
        return 1;
    }

    int value = Memsim_Load(pa);
    printf("The value %d is virtual address %d (physical address %d)\n", value, va, pa);
    return 0;
}
