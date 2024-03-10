#include "types.h"
#include "user.h"
#include "wmap.h"

#define MAP_FIXED       0x0008
#define MAP_SHARED      0x0002
#define MAP_ANONYMOUS   0x0004

int main() {
    uint address = wmap(0x60000000, 8192, MAP_SHARED | MAP_ANONYMOUS, -1);
    if (address == 0) {
        printf(1, "Error: wmap failed\n");
        exit();
    }

    printf(1, "Memory mapped successfully at address: %x\n", address);

    // Access the mapped memory region
    *(int *)address = 123; // Write to the memory

    // Read from the memory and print its value
    printf(1, "Value at mapped address: %d\n", *(int *)address);

    uint anotheraddress = wmap(0x60000000, 8192, MAP_SHARED | MAP_ANONYMOUS, -1);
    if (anotheraddress== 0) {
        printf(1, "Error: wmap failed\n");
        exit();
    }

    printf(1, "Memory mapped successfully at address: %x\n", address);

       // Access the mapped memory region
    *(int *)address = 500; // Write to the memory

    // Read from the memory and print its value
    printf(1, "Value at mapped address: %d\n", *(int *)address);


    uint addr2 = wmap(0x65000000, 8192, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1);
    if (addr2== 0) {
        printf(1, "Error: wmap failed\n");
        exit();
    }

    printf(1, "Memory mapped successfully at address: %x\n", addr2);

    // Access the mapped memory region
    *(int *)addr2 = 2024; // Write to the memory

    // Read from the memory and print its value
    printf(1, "Value at mapped address: %d\n", *(int *)addr2);

    // Print memory mapping information
    struct wmapinfo wminfo;
    if (getwmapinfo(&wminfo) < 0) {
        printf(1, "Error: getwmapinfo failed\n");
        exit();
    }
    printf(1, "Memory Mapping Information:\n");
    for (int i = 0; i < wminfo.total_mmaps; i++) {
        printf(1, "Mapping %d:\n", i+1);
        printf(1, "Address: %x\n", wminfo.addr[i]);
        printf(1, "Length: %d\n", wminfo.length[i]);
        printf(1, "Number of Loaded Pages: %d\n", wminfo.n_loaded_pages[i]);
    }

    if (wunmap(address) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }



    if (wunmap(anotheraddress) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }

    if (wunmap(addr2) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }

    // Test getwmapinfo
    if (getwmapinfo(&wminfo) < 0) {
        printf(1, "Error: getwmapinfo failed\n");
        exit();
    }

    // Print memory mapping information
    printf(1, "Memory Mapping Information:\n");
    for (int i = 0; i < wminfo.total_mmaps; i++) {
        printf(1, "Mapping %d:\n", i+1);
        printf(1, "Address: %x\n", wminfo.addr[i]);
        printf(1, "Length: %d\n", wminfo.length[i]);
        printf(1, "Number of Loaded Pages: %d\n", wminfo.n_loaded_pages[i]);
    }

    exit();
}
