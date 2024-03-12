#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "wmap.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

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

  //Modify your wmap such that it's able to search for an available region in the process address space. This should make your wmap work without MAP_FIXED.

  
  
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

  int new_addr_flag = 0;

  //I need to sort myproc()->my_maps by addr 
  sort_wmapinfo(myproc()->my_maps);
  
  //if MAP_FIXED make sure specified address space is available
  if (addr % PGSIZE != 0 || addr < 0x60000000 || addr > 0x80000000) { 
    if (flags & MAP_FIXED) {
      return FAILED;
    }
    new_addr_flag = 1;
  }
  
  //check free addr space?
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    if (myproc()->my_maps->addr[i] != 0 && // Check only initialized entries
        addr >= myproc()->my_maps->addr[i] &&
        addr < myproc()->my_maps->addr[i] + myproc()->my_maps->length[i]) {
      if (flags & MAP_FIXED){
        return FAILED;
      }
      new_addr_flag = 1;
    } else if (i > 0 && myproc()->my_maps->addr[i-1] != 0 && myproc()->my_maps->addr[i] != 0 &&
              myproc()->my_maps->addr[i-1] + myproc()->my_maps->length[i-1] < addr &&
              myproc()->my_maps->addr[i] > addr){
      if (addr + length > myproc()->my_maps->addr[i]){
        if (flags & MAP_FIXED){
          return FAILED;
        }
        new_addr_flag = 1;
      }
    }
  }
  if (!new_addr_flag){
    for (int i = 0; i < MAX_WMMAP_INFO; i++) {
      if (myproc()->my_maps->addr[i] == 0) {
        myproc()->my_maps->addr[i] = addr;
        myproc()->my_maps->length[i] = length;
        if (!(flags & MAP_ANONYMOUS)) {
          myproc()->my_maps->fd[i] = fd;
        } 
        myproc()->my_maps->total_mmaps++;
	if(flags & MAP_PRIVATE) {
	  myproc()->my_maps->flagPrivate[i] = 1;
	} else {
	  myproc()->my_maps->flagPrivate[i] = 0;
	}
        return myproc()->my_maps->addr[i];
      }
    }
  }else if (new_addr_flag){
    for (int i = 1; i < MAX_WMMAP_INFO; i++) {
      if (myproc()->my_maps->addr[i] == 0) {
        myproc()->my_maps->addr[i] = PGROUNDUP(myproc()->my_maps->addr[i-1] + myproc()->my_maps->length[i-1]);
        myproc()->my_maps->length[i] = length;
        myproc()->my_maps->total_mmaps++;
        if (!(flags & MAP_ANONYMOUS)) {
          myproc()->my_maps->fd[i] = fd;
        }
	if(flags & MAP_PRIVATE) {
          myproc()->my_maps->flagPrivate[i] = 1;
        } else {
          myproc()->my_maps->flagPrivate[i] = 0;
        }
        return myproc()->my_maps->addr[i];
      }
    }
  }
 return FAILED; // No empty slot found
}

int
sys_wunmap(void)
{
  uint addr;

  if (argint(0, (int*)&addr) < 0) {
    return FAILED;
  }
  //TLB flush
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {

    int fd = myproc()->my_maps->fd[i];
    if (myproc()->my_maps->write[i] == 1 && fd >= 0 && fd < NOFILE && myproc()->ofile[fd]) {
      struct file *f = myproc()->ofile[fd];
      f->off = 0;
      begin_op();
      ilock(f->ip);
      writei(f->ip, (char *)addr, f->off, PGSIZE);
      iunlock(f->ip);
      end_op();
    }

    if (addr == myproc()->my_maps->addr[i]) {
      int fd = myproc()->my_maps->fd[i];

      if (myproc()->my_maps->write[i] == 1 && fd >= 0 && fd < NOFILE && myproc()->ofile[fd]){
        struct file *f = myproc()->ofile[fd];
        //int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
        //int i = 0;
        int r;
        f->off = 0;
        //testing something
        //while(i < myproc()->my_maps->length[i]){
         // int n1 = myproc()->my_maps->length[i] - i;
          //if(n1 > max)
           // n1 = max;

          begin_op();
          ilock(f->ip);
          if ((r = writei(f->ip, (char *)addr + PGSIZE, f->off, PGSIZE)) > 0)
            f->off += r;
          iunlock(f->ip);
          end_op();
          //if(r < 0)
           // break;
          //if(r != n1)
            //panic("short filewrite");
          //i += r;
        //}
      }
      uint n_pages = (myproc()->my_maps->length[i] + PGSIZE - 1) / PGSIZE;
      for (int j = 0; j < n_pages; j++) {
        pte_t *pte = walkpgdir(myproc()->pgdir, (void*)(addr + j * PGSIZE), 0);
        if (pte && (*pte & PTE_P)) {
          uint pa = PTE_ADDR(*pte);
          if (pa != 0 && myproc()->parent->pid == 2) // if pa is not null
            kfree(P2V(pa));
          *pte = 0;
          switchuvm(myproc()); //FLUSH THE TLB fixes our issue
          myproc()->my_maps->n_loaded_pages[i]--;
        }
      }
      myproc()->my_maps->write[i] = 0;
      myproc()->my_maps->total_mmaps--;
      myproc()->my_maps->addr[i] = 0;
      myproc()->my_maps->length[i] = 0; 
      myproc()->my_maps->fd[i] = -1;
      myproc()->my_maps->flagPrivate[i] = 0;
      return SUCCESS;
    }
  }
  return FAILED; // Address not found
}

//IMPLEMENT THIS LAST
int
sys_wremap(void)
{

  uint old_addr;
  //only thing is addr and length should have same index
  int old_size, new_size, flags;

  if(argint(0, (int*)&old_addr) < 0) {
    return FAILED;
  }

  if (argint(1, &old_size) < 0 || argint(2, &new_size) < 0 || argint(3, &flags) < 0){
    return FAILED;
  }

  for (int i = 0; i < MAX_WMMAP_INFO; i++){
    if (old_addr == myproc()->my_maps->addr[i] && old_size == myproc()->my_maps->addr[i]) {
    }
  }
  return FAILED;
}

int
sys_getpgdirinfo(void) {
  struct pgdirinfo *pdinfo;
  pde_t *pgdir = myproc()->pgdir; // get the pt for cur proc


  if(argptr(0, (char**) &pdinfo, sizeof(*pdinfo)) < 0) { // check arg1
    return FAILED;
  }
  uint n_upages = 0; // number of user pages

  for(int i = 0; i < NPDENTRIES; i++) {
    uint va;
    uint pa;
    for(int j = 0; j < NPTENTRIES; j++) {
      va = PGADDR(i, j, 0);
      pde_t *pde = &pgdir[i];
      
      if(*pde & PTE_P) {
	pde_t *pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
	pte_t *pte = &pgtab[j];
      	if(!(*pte & PTE_P)) continue;
	
	if(*pte & PTE_U) {
          pa = (uint)(PTE_ADDR(*pte)); // remove offset then add KERNBASE to it?
          
	  if(n_upages < 32) {
            pdinfo->va[n_upages] = va;
            pdinfo->pa[n_upages] = pa;
            n_upages++;
	  }
	}
      }
    }
  }

  pdinfo->n_upages = n_upages;
  return SUCCESS; 
}


int
sys_getwmapinfo(void)
{
  //get pointer
  struct wmapinfo *wminfo;
  if (argptr(0, (char**)&wminfo, sizeof(*wminfo)) < 0)
    return FAILED;

 // Copy memory mapping information to user space
  if (copyout(myproc()->pgdir, (uint)wminfo->addr, myproc()->my_maps->addr, sizeof(myproc()->my_maps->addr)) < 0 ||
      copyout(myproc()->pgdir, (uint)wminfo->length, myproc()->my_maps->length, sizeof(myproc()->my_maps->length)) < 0 ||
      copyout(myproc()->pgdir, (uint)wminfo->n_loaded_pages, myproc()->my_maps->n_loaded_pages, sizeof(myproc()->my_maps->n_loaded_pages)) < 0 ||
      copyout(myproc()->pgdir, (uint)&(wminfo->total_mmaps), &(myproc()->my_maps->total_mmaps), sizeof(myproc()->my_maps->total_mmaps)) < 0 || copyout(myproc()->pgdir, (uint)wminfo->flagPrivate, myproc()->my_maps->flagPrivate, sizeof(myproc()->my_maps->flagPrivate) < 0)) {
        return -1;
  }
  return SUCCESS;
    
}
