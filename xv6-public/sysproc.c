#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "wmap.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//TODO: NEED TO DO SOME DEBUGGING HERE SOME STUFF AINT RIGHT
uint
sys_wmap(void)  
{
  uint addr; 
  int length, flags, fd;
  
  //get addr
  if(argint(0, (int*)&addr) < 0) {
    return FAILED;
  }
  //get other stuff
  if (argint(1, &length) < 0 || argint(2, &flags) < 0 || argint(3, &fd) < 0){
    return FAILED;
  }

  if(myproc()->my_maps->total_mmaps == 16){
    return FAILED;
  }

  myproc()->my_maps->total_mmaps += 1;
  //ADDR NEEDS TO GO IN A SPOT, NOT NECESSARILY n_maps, 
  //because if a spot in middle is freed want to be able to still use it for addr
  //only thing is addr and length should have same index
  for (int i = 0; i < MAX_WMMAP_INFO; i++){
      if (myproc()->my_maps->addr[i] == 0){
        myproc()->my_maps->addr[i] = addr;
        myproc()->my_maps->length[i] = length;
        return addr;
      }
  }

  //might want to make above code and this return clearer idk
  return FAILED;

}

int
sys_wunmap(void)
{
  uint addr;

  if (argint(0, (int*)&addr) < 0) {
    return FAILED;
  }

  //check if address was mapped
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    if (addr == myproc()->my_maps->addr[i]) {
      //remove all pgdir stuff and return
      // TODO: IT WOULD BE BETTER TO DEFINE PAGE SIZE IN SOME HEADER AND USE THAT INSTEAD OF 4096
      uint n_pages = (myproc()->my_maps->length[i] + 4096 - 1) / 4096;
      pte_t *pte;
      uint physical_address;
      for (int j = 0; j < n_pages; j++) {
        pte = walkpgdir(myproc()->pgdir, (void*)(addr + (j * 4096)), 0);
        physical_address = PTE_ADDR(*pte);
        kfree(P2V(physical_address));
        *pte = 0;
      }
      myproc()->my_maps->n_loaded_pages[i] -= n_pages;
      myproc()->my_maps->total_mmaps -= 1;
      myproc()->my_maps->addr[i] = 0;
      myproc()->my_maps->length[i] = 0;
     
      return SUCCESS;
    }
  }
  //Return -1 if addr doesn't exist, since it still failed
  return FAILED;
}
