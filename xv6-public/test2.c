#include "types.h"
#include "user.h"
#include "wmap.h"

#define MAP_FIXED       0x0008
#define MAP_SHARED      0x0002
#define MAP_ANONYMOUS   0x0004

int main() {
    struct pgdirinfo testpgdir;
    if(getpgdirinfo(&testpgdir) < 0) {
        printf(1, "Error: getpgdirinfo failed\n");
        exit();
    }

    printf(1, "n_upages: %d\n", testpgdir.n_upages);
    for(int i = 0; i < ((testpgdir.n_upages > 32) ? 32 : testpgdir.n_upages); i++) {
	printf(1, "va: %p   pa: %p\n", testpgdir.va[i], testpgdir.pa[i]);
    }
    

    uint address = wmap(0x60000000, 4096*4, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1);
    if (address == 0) {
        printf(1, "Error: wmap failed\n");
        exit();
    }

    printf(1, "Memory mapped successfully at address: %x\n", address);

    printf(1, "n_upages: %d\n", testpgdir.n_upages);
    for(int i = 0; i < ((testpgdir.n_upages > 32) ? 32 : testpgdir.n_upages); i++) {
        printf(1, "va: %p   pa: %p\n", testpgdir.va[i], testpgdir.pa[i]);
    }

    // Access the mapped memory region
    *(int *)(address + 1) = 234;
    *(int *)(address + 2) = 1203981;

    // Read from the memory and print its value
    printf(1, "Value at mapped address + 1: %d\n", *(int *)(address + 1));
    printf(1, "Value at mapped address + 2: %d\n", *(int *)(address + 2));

    if (wunmap((uint)address) < 0){
        printf(1, "Error: wunmap failed\n");
        exit();
    }
    printf(1, "This should seg fault?: %d\n", *(int *)(address + 1));

    exit();
}
