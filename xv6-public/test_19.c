#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 19
// Summary: change one small and private filebacked map (checks for memory allocation)
//
// Changes should NOT be reflected in the file
// Checks for memory allocation
// Does not unmap any mapping
// Does not check for lazy allocation
// ====================================================================

char *test_name = "TEST_19";

// TEST HELPER
#define MMAPBASE 0x60000000
#define KERNBASE 0x80000000
#define PGSIZE 4096
#define TRUE 1
#define FALSE 0

void success() {
    printf(1, "\nWMMAP\t SUCCESS\n\n");
    //exit();
}

void failed() {
    printf(1, "\nWMMAP\t FAILED\n\n");
    exit();
}

void get_n_validate_wmap_info(struct wmapinfo *info, int expected_total_mmaps) {
    int ret = getwmapinfo(info);
    if (ret < 0) {
        printf(1, "Cause: `getwmapinfo()` returned %d\n", ret);
        failed();
    }
    if (info->total_mmaps != expected_total_mmaps) {
        printf(1, "Cause: expected %d maps, but found %d\n", expected_total_mmaps, info->total_mmaps);
        failed();
    }
}

void map_exists(struct wmapinfo *info, uint addr, int length, int expected) {
    int found = 0;
    for (int i = 0; i < info->total_mmaps; i++) {
        if (info->addr[i] == addr && info->length[i] == length) {
            found = 1;
            break;
        }
    }
    if (found != expected) {
        printf(1, "Cause: expected 0x%x with length %d to %s in the list of maps\n", addr, length, expected ? "exist" : "not exist");
        failed();
    }
}

int create_small_file(char *filename) {
    // create a file
    int bufflen = 512;
    char buff[bufflen];
    int fd = open(filename, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf(1, "\tCause:\tFailed to create file %s\n", filename);
        failed();
    }
    // prepare the content to write
    for (int j = 0; j < bufflen; j++) {
        buff[j] = 'a';
    }
    // write to file
    if (write(fd, buff, bufflen) != bufflen) {
        printf(1, "Error: Write to file FAILED\n");
        failed();
    }
    close(fd);
    return bufflen;
}

int test_19() {
    printf(1, "\n\n%s\n", test_name);

    // validate initial state
    struct wmapinfo winfo1;
    get_n_validate_wmap_info(&winfo1, 0); // no maps exist
    printf(1, "Initially 0 maps. \tOkay.\n");

    // create and open a small file
    char *filename = "small.txt";
    int filelength = create_small_file(filename);
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf(1, "Cause: Failed to open file %s\n", filename);
        failed();
    }
    struct stat st;
    if (fstat(fd, &st) < 0) {
        printf(1, "Cause: Failed to get file stat\n");
        failed();
    }
    if (st.size != filelength) {
        printf(1, "Cause: File size %d != %d\n", st.size, filelength);
        failed();
    }
    printf(1, "Created file %s with length %d. \tOkay.\n", filename, filelength);

    // place map 1 (fixed and filebacked)
    int fixed_filebacked_priv = MAP_FIXED | MAP_PRIVATE;
    uint addr = 0x60021000;
    uint length = filelength;
    uint map = wmap(addr, length, fixed_filebacked_priv, fd);
    if (map != addr) {
        printf(1, "Cause: `wmap()` returned %d\n", map);
        failed();
    }
    // validate mid state
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1);   // 1 map exists
    map_exists(&winfo2, map, length, TRUE); // map 1 exists
    printf(1, "Placed map 1 at 0x%x with length %d. \tOkay.\n", map, length);

    // validate all pages of map 1
    char *arr = (char *)map;
    char oldval = 'a';
    for (int i = 0; i < length; i++) {
        if (arr[i] != oldval) {
            printf(1, "expected '%c', but got '%c'\n", oldval, arr[i]);
            failed();
        }
    }
    printf(1, "All pages of map 1 are as expected. \tOkay.\n");

    // change all pages of map 1
    char val = 'p';
    for (int i = 0; i < length; i++) {
        arr[i] = val;
    }
    printf(1, "Changed all pages of map 1 to '%c'. \tOkay.\n", val);

    // unmap the map
    int ret = wunmap(map);
    if (ret < 0) {
        printf(1, "Cause: `wunmap()` returned %d\n", ret);
        failed();
    }
    // validate final state
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 0); // no maps exist
    printf(1, "Unmapped map 1. \tOkay.\n");
    close(fd); // close the files

    // reopen the file and validate its content
    fd = open(filename, 'r');
    if (fd < 0) {
        printf(1, "Cause: Failed to open file %s\n", filename);
        failed();
    }
    int bufflen = filelength;
    char buff[bufflen + 1];
    if (read(fd, buff, bufflen) != bufflen) {
        printf(1, "Error: Read from file %s FAILED\n", filename);
        failed();
    }
    for (int j = 0; j < bufflen; j++) {
        if (buff[j] != oldval) {
            printf(1, "expected '%c', but got '%c'\n", oldval, buff[j]);
            failed();
        }
    }
    close(fd);
    printf(1, "File content is as expected. \tOkay.\n");

    // test ends
    success();
    return 1;
}
int main() {
    test_19();
    test_19();
    test_19();
    exit();
}
