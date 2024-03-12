#include "types.h"
#include "wmap.h"
#include "user.h"
#include "fcntl.h"


void test_fork(void) {
  printf(1, "Testing fork:\n");
  int pid = 0;
  printf(1, "\t-0x60000000 is shared\n");
  uint map0 = wmap(0x60000000, 4096, MAP_FIXED | MAP_SHARED, -1);
  *(int*)map0 = 1;
  uint map1 = wmap(0x60001000, 4096, MAP_FIXED | MAP_PRIVATE, -1);
  printf(1, "\t-0x60001000 is private\n");
  map1 = map1;
  *(int*)map1 = 2;
  pid = fork();
  if (pid == 0) { //child
    struct pgdirinfo pd;
    getpgdirinfo(&pd);
    for(int i = 0; i < 32; ++i) {
      //printf(1, "CHILD:: va[%d] 0x%x, pa 0x%x\n",i, pd.va[i], pd.pa[i]);
    }
    *(int*)map1 = 3;
    *(int*)map0 = 4;
    printf(1, "CHILD:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    int o = 0;
    for(int k = 0; k < 100000000; ++k) {
      k = k;
      ++o;
    }
    printf(1, "finished\n");
  } else { //parent
    struct pgdirinfo pd;
    getpgdirinfo(&pd);
    for(int i = 0; i < 32; ++i) {
      //printf(1, "PARENT:: va[%d] 0x%x, pa 0x%x\n",i, pd.va[i], pd.pa[i]);
    }
    *(int*)map1 = 5;
    *(int*)map0 = 6;
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
    printf(1, "PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);

    wait();
    printf(1, "LAST __ PARENT:: map0 %d, map1 %d\n", *(int*)map0, *(int*)map1);
  }
  return;
}
