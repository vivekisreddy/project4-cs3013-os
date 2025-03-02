// Starting code version 1.0

#include <assert.h>
#include <string.h>

#include "memsim.h"

/* Private Internals: */

// Currently 'boolean' array (bitmap would be better for scale and perf)
short freePages[NUM_PAGES];

// The simulated physical memory array (in bytes), aka physical R.A.M.
char physmem[PHYSICAL_SIZE];

/*
 * Performs sanity checks based on the simulations defined constants.
 * If these checks fail, the simulation is not valid and should not proceed.
 * Asserts are used to halt the program immediately if these checks fail.
 */
void MemsimConfigSanityChecks() { // you should not modify this function
	assert(PHYSICAL_SIZE % PAGE_SIZE == 0); // physical size in bytes must be a multiple of page size bytes
	assert(VIRTUAL_SIZE == PHYSICAL_SIZE); // standard implementation gives whole PA space to each process (virtually)
    assert(MAX_VA == MAX_PA);
    assert(NUM_PAGES == NUM_FRAMES);
}

/*
 *  Public Interface: 
 */

/* 
 * Zeroes out the simulated physical memory array.
 */
void Memsim_Init() { // zero free pages list 
    MemsimConfigSanityChecks();
    memset(physmem, 0, sizeof(physmem)); // zero out physical memory
	memset(freePages, 0, sizeof(freePages)); // zero implies free / FALSE / not used
}

 /* Gets current shared reference to start of simulated physical memory. */
char* Memsim_GetPhysMem() {
    return physmem;
}

/*
 * Searches through the free page list to find the first free page in memory. 
 * It claims the page, marking it, and returns the physical address of the beginning 
 * of the page. If there are no free pages, returns -1;
 */
int Memsim_FirstFreePFN() {
    for (int i = 1; i < NUM_PAGES; i++) {  
        if (freePages[i] == 0) {
            printf("Allocating frame %d\n", i);  // Debug print
            freePages[i] = 1;
            return i;
        }
    }
    return -1;
}



void Memsim_Store(int physical_address, int value) {
    char* physmem = Memsim_GetPhysMem();
    physmem[physical_address] = (char)value;
}

int Memsim_Load(int physical_address) {
    char* physmem = Memsim_GetPhysMem();
    return (int)(unsigned char)physmem[physical_address];
}

// Swap storage (assuming a simple array for swap slots)
#define SWAP_SIZE  NUM_PAGES
char swap_space[SWAP_SIZE * PAGE_SIZE];  // Simulated disk swap space
int swap_slots[SWAP_SIZE];               // Keeps track of used swap slots

int Memsim_SwapOut(int frame_number) {
    for (int i = 0; i < SWAP_SIZE; i++) {
        if (swap_slots[i] == 0) { 
            memcpy(&swap_space[i * PAGE_SIZE], &physmem[frame_number * PAGE_SIZE], PAGE_SIZE);
            swap_slots[i] = 1;
            freePages[frame_number] = 0; 
            return i;
        }
    }
    return -1;
}

int Memsim_SwapIn(int pid, int vpn, int swap_slot) {
    if (swap_slot < 0 || swap_slot >= SWAP_SIZE || swap_slots[swap_slot] == 0) {
        return -1;
    }

    int new_frame = Memsim_FirstFreePFN();
    if (new_frame == -1) return -1;

    memcpy(&physmem[new_frame * PAGE_SIZE], &swap_space[swap_slot * PAGE_SIZE], PAGE_SIZE);
    swap_slots[swap_slot] = 0;

    PT_SetPTE(pid, vpn, new_frame, 1, 1, 1, 0);
    return new_frame;
}
