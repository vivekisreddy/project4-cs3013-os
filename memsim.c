#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PHYSICAL_MEMORY_SIZE 64 // Total size of physical memory in bytes
#define PAGE_SIZE 16           // Size of a page (in bytes)
#define NUM_PAGES (PHYSICAL_MEMORY_SIZE / PAGE_SIZE) // Number of pages
#define NUM_PROCESSES 4        // Number of processes
#define VIRTUAL_MEMORY_SIZE 64 // Virtual memory space size in bytes
#define NUM_VIRTUAL_PAGES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE) // Virtual pages per process

// Define a structure for Page Table Entries (PTE)
typedef struct {
    int PFN;        // Physical Frame Number
    int valid;      // Valid bit
    int protection; // Protection bit (1 = writable, 0 = read-only)
} PTE;

// Physical memory simulated as an array of bytes
unsigned char memory[PHYSICAL_MEMORY_SIZE];

// Array of registers pointing to the page table for each process
int pageTableRegisters[NUM_PROCESSES];

// Function to initialize memory
void init_memory() {
    for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
        memory[i] = 0; // Initialize all bytes to 0
    }
}

// Function to initialize page table registers
void init_page_table_registers() {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pageTableRegisters[i] = -1; // No page table assigned initially
    }
}

// Function to allocate a new page table for a process
void create_page_table(int pid) {
    if (pageTableRegisters[pid] != -1) {
        return; // Page table already exists
    }

    // Find a free physical frame for the page table
    for (int i = 0; i < NUM_PAGES; i++) {
        if (memory[i * PAGE_SIZE] == 0) { // Check if the page is free
            pageTableRegisters[pid] = i * PAGE_SIZE;
            printf("Put page table for PID %d into physical frame %d\n", pid, i);
            return;
        }
    }

    // If no free physical page for the page table, print an error
    printf("Error: No free physical page for process %d\n", pid);
}

// Function to map a virtual address to a physical address
void map(int pid, int virtual_address, int value) {
    // Check if the page table exists for the process
    if (pageTableRegisters[pid] == -1) {
        create_page_table(pid);
    }

    int virtual_page_number = virtual_address / PAGE_SIZE;
    int physical_page_number = -1;

    // Find a free physical page
    for (int i = 0; i < NUM_PAGES; i++) {
        if (memory[i * PAGE_SIZE] == 0) { // Check if the page is free
            physical_page_number = i;
            break;
        }
    }

    if (physical_page_number == -1) {
        printf("Error: No free physical page for mapping\n");
        return;
    }

    // Place the page table entry in the page table
    PTE* page_table = (PTE*)(memory + pageTableRegisters[pid]);
    page_table[virtual_page_number].PFN = physical_page_number;
    page_table[virtual_page_number].valid = 1;
    page_table[virtual_page_number].protection = value;

    // Print the mapping result
    printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtual_address, virtual_page_number, physical_page_number);
}

// Function to perform store operation (write to memory)
void store(int pid, int virtual_address, int value) {
    if (pageTableRegisters[pid] == -1) {
        create_page_table(pid);
    }

    int virtual_page_number = virtual_address / PAGE_SIZE;
    PTE* page_table = (PTE*)(memory + pageTableRegisters[pid]);

    if (page_table[virtual_page_number].valid == 0) {
        printf("Error: Invalid page for process %d\n", pid);
        return;
    }

    if (page_table[virtual_page_number].protection == 0) {
        printf("Error: Write to read-only page for process %d\n", pid);
        return;
    }

    int physical_frame_number = page_table[virtual_page_number].PFN;
    int physical_address = physical_frame_number * PAGE_SIZE + (virtual_address % PAGE_SIZE);

    memory[physical_address] = value; // Store the value in physical memory
    printf("Stored value %d at virtual address %d (physical address %d)\n", value, virtual_address, physical_address);
}

// Function to perform load operation (read from memory)
void load(int pid, int virtual_address) {
    if (pageTableRegisters[pid] == -1) {
        create_page_table(pid);
    }

    int virtual_page_number = virtual_address / PAGE_SIZE;
    PTE* page_table = (PTE*)(memory + pageTableRegisters[pid]);

    if (page_table[virtual_page_number].valid == 0) {
        printf("Error: Invalid page for process %d\n", pid);
        return;
    }

    int physical_frame_number = page_table[virtual_page_number].PFN;
    int physical_address = physical_frame_number * PAGE_SIZE + (virtual_address % PAGE_SIZE);

    printf("The value %d is at virtual address %d (physical address %d)\n", memory[physical_address], virtual_address, physical_address);
}

int main() {
    init_memory();
    init_page_table_registers();

    // Sample input: process_id,instruction_type,virtual_address,value
    char instruction[100];
    while (1) {
        printf("Instruction? ");
        fgets(instruction, sizeof(instruction), stdin);

        int pid, virtual_address, value;
        char instruction_type[10];

        if (sscanf(instruction, "%d,%9s,%d,%d", &pid, instruction_type, &virtual_address, &value) != 4) {
            printf("Invalid instruction format.\n");
            continue;
        }

        if (pid < 0 || pid >= NUM_PROCESSES) {
            printf("Error: Invalid process ID.\n");
            continue;
        }

        if (virtual_address < 0 || virtual_address >= VIRTUAL_MEMORY_SIZE) {
            printf("Error: Invalid virtual address.\n");
            continue;
        }

        if (strcmp(instruction_type, "map") == 0) {
            map(pid, virtual_address, value);
        } else if (strcmp(instruction_type, "store") == 0) {
            store(pid, virtual_address, value);
        } else if (strcmp(instruction_type, "load") == 0) {
            load(pid, virtual_address);
        } else {
            printf("Error: Invalid instruction type.\n");
        }
    }

    return 0;
}
