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

// NEED TO IMPLEMENT BASIC WMAP
uint
sys_wmap(void)  
{
  uint addr; 
  int length, flags, fd;
  
  //get addr
  if(argint(0, (int*)&addr) < 0) {
    return (uint)-1;
  }
  //get other stuff
  if (argint(1, &length) < 0 || argint(2, &flags) < 0 || argint(3, &fd) < 0){
    return (uint)-1;
  }

  //add to struct, information but don't physicall asign pages
  if(myproc()->my_maps->total_mmaps == 16){
    return (uint)-1;
  }

  int n_maps = ++myproc()->my_maps->total_mmaps;
  myproc()->my_maps->addr[n_maps - 1] = addr;
  myproc()->my_maps->length[n_maps - 1] = length;



 // return (uint)-1;
  return addr;
}

int
sys_wunmap(void)
{
  uint addr;

  if (argint(0, (int*)&addr) < 0) {
    return (uint)-1;
  }

  return 0;
}
