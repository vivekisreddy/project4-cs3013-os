// Starting code version 1.1

/*
 * Public Interface:
 */

#define NUM_PROCESSES 4

/*
 * Public Interface:
 */
void PT_SetPTE(int process_id, int VPN, int PFN, int valid, int protection, int present, int referenced);
int PT_PageTableInit(int process_id, int pageAddress);
int PT_PageTableExists(int process_id);
int PT_GetRootPtrRegVal(int process_id);
int PT_Evict();
int PT_VPNtoPA(int process_id, int VPN);
int PT_PIDHasWritePerm(int process_id, int VPN);
void PT_Init();
int PT_Map(int process_id, int VPN, int frame);
void PT_PageTableCreate(int pid, int pfn);
int PT_GetWritePerm(int pid, int vpn);
void PT_UpdateWritePerm(int pid, int vpn, int new_perm);
typedef struct {
    int PFN;         // Page frame number
    int valid;       // Valid bit
    int protection;  // Read/write permissions
    int present;     // In memory or swapped out
    int referenced;  // Last access or swap offset
    int rw_bit;      // Read/Write permission bit
} PageTableEntry;

PageTableEntry* PT_GetPTE(int pid, int vpn); // Function prototype
