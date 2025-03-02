// Starting code version 1.0

#include <assert.h>
#include <string.h>
#include <stdio.h>

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
int Memsim_FirstFreePFN(){
    for(int i = 0; i < NUM_PAGES; i++){
        if (freePages[i] == 0){
            freePages[i] = 1;
            return i * PAGE_SIZE;
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

#define SWAP_SLOT_COUNT 100  // Large enough swap space
int swapSlots[SWAP_SLOT_COUNT] = {0};  // 0 = free, 1 = occupied

int Memsim_EvictPage() {
    static int evictionIndex = 0;  // Round-robin index
    for (int i = 0; i < NUM_PAGES; i++) {
        int frame = (evictionIndex + i) % NUM_PAGES;
        if (freePages[frame] == 1) {  // If page is in use
            // Find a free swap slot
            int swapSlot = -1;
            for (int j = 0; j < SWAP_SLOT_COUNT; j++) {
                if (swapSlots[j] == 0) {
                    swapSlot = j;
                    swapSlots[j] = 1; // Mark as occupied
                    break;
                }
            }
            if (swapSlot == -1) {
                printf("Error: No available swap space.\n");
                return -1;
            }
            // Write page to swap file
            FILE* swapFile = fopen("disk.txt", "r+");
            if (swapFile) {
                fseek(swapFile, swapSlot * PAGE_SIZE, SEEK_SET);
                fwrite(&physmem[frame * PAGE_SIZE], PAGE_SIZE, 1, swapFile);
                fclose(swapFile);
            }
            // Mark page as swapped
            PT_SetSwapped(frame, swapSlot);
            freePages[frame] = 0;
            evictionIndex = (frame + 1) % NUM_PAGES;  // Move to next for round-robin
            return frame * PAGE_SIZE;
        }
    }
    return -1;
}
