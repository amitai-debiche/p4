#include "types.h"
#include "user.h"

#define MAP_FIXED       0x0008
#define MAP_SHARED      0x0002
#define MAP_ANONYMOUS   0x0004

int main() {
    uint address = wmap(0x60000000, 8192, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1);
    if (address == 0) {
        printf(1, "Error: wmap failed\n");
        exit();
    }

    printf(1, "Memory mapped successfully at address: %x\n", address);

    // Access the mapped memory region
    *(int *)address = 123; // Write to the memory

    // Read from the memory and print its value
    printf(1, "Value at mapped address: %d\n", *(int *)address);

    if (wunmap(address) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }

    address = wmap(0x60000000, 8192, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1);
    if (address == 0) {
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

     if (wunmap(address) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }

     if (wunmap(addr2) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }

    printf(1, "This should seg fault?: %d\n", *(int *)address);

    exit();
}
