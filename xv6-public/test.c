#include "types.h"
#include "stat.h"
#include "user.h"

#define SIZE (4096 * 10)  // Size of the memory region to map (10 pages)

int
main(int argc, char *argv[])
{
    char *addr;

    // Map a memory region using wmap
    addr = wmap(0, SIZE, 0, 0);

    // Check if wmap succeeded
    if (addr == (char *)FAILED) {
        printf(1, "wmap failed\n");
        exit();
    }

    // Access the memory region to trigger page faults
    printf(1, "Accessing mapped memory...\n");
    for (int i = 0; i < SIZE; i++) {
        // Access memory (read)
        char value = addr[i];
    }

    printf(1, "Mapped memory accessed successfully\n");

    // Exit
    exit();
}
