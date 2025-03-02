// Starting code version 1.0

#ifndef MEMSIM_H
#define MEMSIM_H

#include <assert.h>

/*
 * Public Interface:
 */

#define PAGE_SIZE 16

#define PHYSICAL_SIZE 64
#define VIRTUAL_SIZE 64 

#define MAX_VA 255
#define MAX_PA 255

#define NUM_PAGES (PHYSICAL_SIZE/PAGE_SIZE)
#define NUM_FRAMES NUM_PAGES

// Address getters
#define PAGE_START(i) (i * PAGE_SIZE)
#define PAGE_OFFSET(addr) (addr % PAGE_SIZE)
#define VPN(addr) (addr / PAGE_SIZE)
#define PFN(addr) (addr / PAGE_SIZE)
#define PAGE_NUM(addr) (addr / PAGE_SIZE)

// Public functions 
void Memsim_Init();
char* Memsim_GetPhysMem();
int Memsim_FirstFreePFN();
// Add these missing function declarations
void Memsim_Store(int physical_address, int value);
int Memsim_Load(int physical_address);
void PT_SetSwapped(int frame, int swapSlot);
int Memsim_EvictPage();



#endif // MEMSIM_H