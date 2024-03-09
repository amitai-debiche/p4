#include "types.h"
#include "user.h"

#define MAP_FIXED       0x0008
#define MAP_SHARED      0x0002
#define MAP_ANONYMOUS   0x0004

int main() {
    int *address = (int*)wmap(0x60000000, 4096*4, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1);
    if (address == 0) {
        printf(1, "Error: wmap failed\n");
        exit();
    }

    printf(1, "Memory mapped successfully at address: %x\n", address);

    // Access the mapped memory region
    *(address + 1) = 234;

    // Read from the memory and print its value
    printf(1, "Value at mapped address: %d\n", *(int *)(address + 1));

    if (wunmap((uint)address) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }
    printf(1, "This should seg fault?: %d\n", *(int *)(address + 1));

    exit();
}
