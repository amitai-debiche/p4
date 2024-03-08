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

    exit();
}
