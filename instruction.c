#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_VA 63          // Maximum virtual address (0 to 63)
#define PAGE_SIZE 16       // Page size in bytes
#define NUM_PAGES 4        // Number of pages in physical memory
#define NUM_VIRTUAL_PAGES 4 // Number of virtual pages per process

#define MAX_PROCESSES 4    // Maximum number of processes (PID 0 to 3)

// Memory simulation structure
char physmem[NUM_PAGES * PAGE_SIZE];  // Simulated physical memory
int page_table_register[MAX_PROCESSES]; // Register pointing to page tables for each PID

// Page Table entry structure (VPN -> PFN, with permission)
typedef struct {
    int pfn;  // Physical frame number
    int permission;  // 0: read-only, 1: read-write
} PageTableEntry;

// Page table for each process (each process has its own table in physical memory)
PageTableEntry* page_tables[MAX_PROCESSES];

// Helper functions
int VPN(int va) {
    return va / PAGE_SIZE;  // Convert virtual address to virtual page number
}

int PAGE_OFFSET(int va) {
    return va % PAGE_SIZE;  // Get offset within the page
}

int PFN(int pa) {
    return pa / PAGE_SIZE;  // Convert physical address to physical frame number
}

// Function to get the first free physical frame number
int Memsim_FirstFreePFN() {
    for (int i = 0; i < NUM_PAGES; i++) {
        for (int j = 0; j < PAGE_SIZE; j++) {
            if (physmem[i * PAGE_SIZE + j] == 0) {
                return i;  // Return the first free page frame number
            }
        }
    }
    return -1; // No free physical frame
}

// Function to allocate physical page for a process's page table
int AllocatePageTable(int pid) {
    int frame = Memsim_FirstFreePFN();
    if (frame == -1) {
        printf("Error: No free physical frames available for PID %d's page table.\n", pid);
        return -1;
    }

    page_tables[pid] = (PageTableEntry*)&physmem[frame * PAGE_SIZE];
    page_table_register[pid] = frame;
    printf("Put page table for PID %d into physical frame %d.\n", pid, frame);
    return frame;
}

// Function to map virtual address to physical address for a process
int Instruction_Map(int pid, int va, int value_in) {
    if (value_in != 0 && value_in != 1) {
        printf("Invalid value for map instruction. Value must be 0 or 1.\n");
        return 1;
    }

    // Check if the page table exists, if not, create it
    if (page_tables[pid] == NULL) {
        if (AllocatePageTable(pid) == -1) {
            return 1; // Error allocating page table
        }
    }

    int vpn = VPN(va);
    // Check if the page is already mapped
    if (page_tables[pid][vpn].pfn != -1) {
        printf("Error: Virtual page %d already mapped into physical frame %d.\n", vpn, page_tables[pid][vpn].pfn);
        return 1;
    }

    // Find a free physical frame and map it to the virtual page
    int pa = Memsim_FirstFreePFN();
    if (pa == -1) {
        printf("Error: No free physical frames available.\n");
        return 1;
    }

    // Map virtual page to physical page with permission
    page_tables[pid][vpn].pfn = pa;
    page_tables[pid][vpn].permission = value_in;

    printf("Mapped virtual address %d (page %d) into physical frame %d.\n", va, vpn, pa);
    return 0;
}

// Function to store value in a virtual address
int Instruction_Store(int pid, int va, int value_in) {
    int vpn = VPN(va);

    // Check if the page table exists
    if (page_tables[pid] == NULL) {
        printf("Error: No page table for PID %d.\n", pid);
        return 1;
    }

    // Check if the page is writable
    if (page_tables[pid][vpn].permission == 0) {
        printf("Error: Virtual address %d is read-only.\n", va);
        return 1;
    }

    // Translate virtual address to physical address
    int pa = page_tables[pid][vpn].pfn * PAGE_SIZE + PAGE_OFFSET(va);
    if (pa < 0 || pa >= NUM_PAGES * PAGE_SIZE) {
        printf("Error: Invalid physical address %d.\n", pa);
        return 1;
    }

    // Store the value in physical memory
    physmem[pa] = value_in;
    printf("Stored value %u at virtual address %d (physical address %d)\n", value_in, va, pa);
    return 0;
}

// Function to load value from a virtual address
int Instruction_Load(int pid, int va) {
    int vpn = VPN(va);

    // Check if the page table exists
    if (page_tables[pid] == NULL) {
        printf("Error: No page table for PID %d.\n", pid);
        return 1;
    }

    // Translate virtual address to physical address
    int pa = page_tables[pid][vpn].pfn * PAGE_SIZE + PAGE_OFFSET(va);
    if (pa < 0 || pa >= NUM_PAGES * PAGE_SIZE) {
        printf("Error: Invalid physical address %d.\n", pa);
        return 1;
    }

    // Load the value from physical memory
    uint8_t value = physmem[pa];
    printf("The value %u is at virtual address %d (physical address %d)\n", value, va, pa);
    return 0;
}

int main() {
    // Example usage
    // Input: PID 0, map virtual address 0, with write permission (1)
    Instruction_Map(0, 0, 1);
    // Input: Store value 24 at virtual address 12 for PID 0
    Instruction_Store(0, 12, 24);
    // Input: Load value from virtual address 12 for PID 0
    Instruction_Load(0, 12);

    return 0;
}
