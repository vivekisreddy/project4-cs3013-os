#include <stdio.h>
#include <stdint.h>

#include "memsim.h"
#include "pagetable.h"
#include "mmu.h"

int Instruction_Map(int pid, int va, int value_in) {
    int vpn = VPN(va);
    PageTableEntry* pte = PT_GetPTE(pid, vpn);

    if (pte != NULL) { // If page is already mapped
        if (pte->rw_bit == 0 && value_in == 1) {
            PT_UpdateWritePerm(pid, vpn, 1);
            printf("Updating permissions for virtual page %d (frame %d)\n", vpn, pte->PFN);
        } else {
            printf("Error: virtual page %d is already mapped with rw_bit=%d\n", vpn, pte->rw_bit);
        }
        return 0;  
    }

    // Ensure page table exists
    if (!PT_PageTableExists(pid)) {
        int pt_frame = (pid == 0) ? 0 : 1;
        PT_PageTableCreate(pid, pt_frame);
        printf("Put page table for PID %d into physical frame %d\n", pid, pt_frame);
    }

    int frame = Memsim_FirstFreePFN();
    if (frame == -1) {
        frame = PT_Evict(); // Evict if necessary
    }

    PT_SetPTE(pid, vpn, frame, 1, value_in, 1, 0);
    printf("Mapped virtual address %d (page %d) into physical frame %d\n", va, vpn, frame);
    return 0;
}



int Instruction_Store(int pid, int va, int value_in) {
    int vpn = VPN(va);
    int offset = PAGE_OFFSET(va);
    int pa = MMU_TranslateAddress(pid, vpn, offset);

    if (pa == -1) {
        printf("Error: Virtual address %d is not mapped.\n", va);
        return 1;
    }

    if (!PT_GetWritePerm(pid, vpn)) {  // If write permission is not allowed
        printf("Error: writes are not allowed to this page\n");
        return 1;
    }

    Memsim_Store(pa, value_in);
    printf("Stored value %u at virtual address %d (physical address %d)\n", value_in, va, pa);
    return 0;
}


int Instruction_Load(int pid, int va) {
    int vpn = VPN(va);
    int offset = PAGE_OFFSET(va);
    int pa = MMU_TranslateAddress(pid, vpn, offset);

    if (pa == -1) { // If page is not in memory, swap it in
        int evicted_frame = PT_Evict();
        int swap_slot = Memsim_SwapOut(evicted_frame);
        printf("Swapped frame %d to disk at swap slot %d\n", evicted_frame, swap_slot);

        int new_frame = Memsim_SwapIn(pid, vpn, swap_slot);
        printf("Swapped disk slot %d into frame %d\n", swap_slot, new_frame);
        pa = MMU_TranslateAddress(pid, vpn, offset);
    }

    int value = Memsim_Load(pa);
    printf("The value %d is virtual address %d (physical address %d)\n", value, va, pa);
    return 0;
}
