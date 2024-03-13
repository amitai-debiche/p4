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

    if (addr == myproc()->my_maps->addr[i]) {
      int fd = myproc()->my_maps->fd[i];
     if (myproc()->my_maps->flagPrivate[i] == 0 && myproc()->my_maps->write[i] == 1 && fd >= 0 && fd < NOFILE && myproc()->ofile[fd]) {
        struct file *f = myproc()->ofile[fd];
        f->off = 0;
        int r;
        int num_pages = PGROUNDDOWN(myproc()->my_maps->length[i]) / 4096;
        for (int k = 0; k < num_pages; k++) {
          begin_op();
          ilock(f->ip);
          if ((r = writei(f->ip, (char *)addr + i, f->off, PGSIZE)) > 0)
            f->off += r;
          iunlock(f->ip);
          end_op();
        }
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

// flags = 0 or MREMAP_MAYMOVE
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

  sort_wmapinfo(myproc()->my_maps); // sort in ascending order, zero at the end of the map

  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    if (old_addr == myproc()->my_maps->addr[i] && old_size == myproc()->my_maps->length[i]) {
      if (new_size <= old_size) { // shrinking will stay in the same place
        for(int j = 0; j < PGROUNDDOWN(old_size - new_size); j += PGSIZE) {
	  // might need to add FD here? I am not sure
	  uint addr = PGROUNDUP(old_addr + new_size) + j;
	  pte_t *pte = walkpgdir(myproc()->pgdir, (void*)addr, 0);
	  if(pte && (*pte & PTE_P)) {
	    uint pa = PTE_ADDR(*pte);

	    if (pa != 0 && myproc()->parent->pid != 2) {
	      kfree(P2V(pa));
	    }
	    *pte = 0;
	    switchuvm(myproc());
	  }
	}
	myproc()->my_maps->length[i] = new_size;
	return myproc()->my_maps->addr[i];
      } else { // new_size > old_size
        // cprintf("New_size > old_size\n");
	
	// if the space is not big enough
	if((i + 1 < MAX_WMMAP_INFO && myproc()->my_maps->addr[i + 1] != 0 && myproc()->my_maps->addr[i + 1] <= PGROUNDUP(myproc()->my_maps->addr[i] + new_size)) ||  
			// if the next one is not big enough
	    (i == MAX_WMMAP_INFO && PGROUNDUP(myproc()->my_maps->addr[i] + new_size) >= KERNBASE) || 
	                // or last one to KERNBASE aren't big enough
	    (myproc()->my_maps->addr[0] != 0 && myproc()->my_maps->addr[0] - 0x60000000 < PGROUNDUP(new_size))) {
		        // or 0x60000000 to first one aren't big enough 
	  // cprintf("Space not big enough\n"); 
	  
          if(flags == 0) { // map may not move
	    return FAILED;
	  } else if(flags == MREMAP_MAYMOVE) { // map may move
	    for(int j = 0; j < MAX_WMMAP_INFO; j++) {
	      if(j == MAX_WMMAP_INFO - 1 || myproc()->my_maps->addr[j + 1] == 0) { // next one is not available, then calculate till KERNBASE
		if(PGROUNDUP(myproc()->my_maps->addr[j] + myproc()->my_maps->length[j]) + PGROUNDUP(new_size) < KERNBASE) {
		  
		uint base = PGROUNDUP(myproc()->my_maps->addr[j] + myproc()->my_maps->length[j]);
	       	for(int k = 0; k < new_size; k += PGSIZE) {
	            uint new_va = base + k;
		    uint old_va = old_addr + k;
		    pte_t *old_pte = walkpgdir(myproc()->pgdir, (void*)old_va, 0);
		    //pte_t *new_pte = walkpgdir(myproc()->pgdir, (void*)new_va, 0);
		    if (old_pte && (*old_pte & PTE_P)) {
	              uint old_pa = PTE_ADDR(*old_pte);
		      //uint new_pa = PTE_ADDR(*new_pte);
                      char *mem = kalloc();
	    	      if (!mem) return FAILED;
	              memmove((void*)mem, (void*)(P2V(old_pa)), PGSIZE);
	              if (mappages(myproc()->pgdir, (void*)PGROUNDDOWN(new_va), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
                        kfree(mem);
            	        return FAILED;
	              }
		      if(myproc()->parent->pid == 2) kfree(P2V(old_pa));
		      *old_pte = 0;
		      switchuvm(myproc());
		    }
		  }
                  
		  myproc()->my_maps->addr[i] = PGROUNDUP(myproc()->my_maps->addr[j] + myproc()->my_maps->length[j]);
                  myproc()->my_maps->length[i] = new_size;
                  return myproc()->my_maps->addr[i];
                }
	      }
	      if(myproc()->my_maps->addr[j + 1] - PGROUNDUP(myproc()->my_maps->addr[j] + myproc()->my_maps->length[j]) >= PGROUNDUP(new_size)) {
	        // if this space is big enough
		uint base = PGROUNDUP(myproc()->my_maps->addr[j] + myproc()->my_maps->length[j]);
                for(int k = 0; k < new_size; k += PGSIZE) {
                  uint new_va = base + k;
                  uint old_va = old_addr + k;
                  pte_t *old_pte = walkpgdir(myproc()->pgdir, (void*)old_va, 0);
                  //pte_t *new_pte = walkpgdir(myproc()->pgdir, (void*)new_va, 0);
                  if (old_pte && (*old_pte & PTE_P)) {
                    uint old_pa = PTE_ADDR(*old_pte);
                    //uint new_pa = PTE_ADDR(*new_pte);
                    char *mem = kalloc();
                    if (!mem) return FAILED;
                    memmove((void*)mem, (void*)(P2V(old_pa)), PGSIZE);
                    if (mappages(myproc()->pgdir, (void*)PGROUNDDOWN(new_va), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
                      kfree(mem);
                      return FAILED;
                    }
                    if(myproc()->parent->pid == 2) kfree(P2V(old_pa));
                    *old_pte = 0;
                    switchuvm(myproc());
                  }
                }

		myproc()->my_maps->addr[i] = PGROUNDUP(myproc()->my_maps->addr[j] + myproc()->my_maps->length[j]);
		myproc()->my_maps->length[i] = new_size;
		return myproc()->my_maps->addr[i];
	      }
	      // Need to check the 0x60000000 till first mapping as well
	      if (myproc()->my_maps->addr[0] - 0x60000000 >= PGROUNDUP(new_size)) {
	        uint base = 0x60000000;
                for(int k = 0; k < new_size; k += PGSIZE) {
                  uint new_va = base + k;
                  uint old_va = old_addr + k;
                  pte_t *old_pte = walkpgdir(myproc()->pgdir, (void*)old_va, 0);
                  //pte_t *new_pte = walkpgdir(myproc()->pgdir, (void*)new_va, 0);
                  if (old_pte && (*old_pte & PTE_P)) {
                    uint old_pa = PTE_ADDR(*old_pte);
                    //uint new_pa = PTE_ADDR(*new_pte);
                    char *mem = kalloc();
                    if (!mem) return FAILED;
                    memmove((void*)mem, (void*)(P2V(old_pa)), PGSIZE);
                    if (mappages(myproc()->pgdir, (void*)PGROUNDDOWN(new_va), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
                      kfree(mem);
                      return FAILED;
                    }
                    if(myproc()->parent->pid == 2) kfree(P2V(old_pa));
                    *old_pte = 0;
                    switchuvm(myproc());
                    }
                } 
		myproc()->my_maps->addr[i] = 0x60000000;
		myproc()->my_maps->length[i] = new_size;
                return myproc()->my_maps->addr[i];
	      }
	    }
	    return FAILED; // either its the last one or it doesn't work
	  }
	} else { // if space is big enough
	  // regardless of movable or not, we can return the same mapping
	  // cprintf("Space is indeed big enough\n");
	  myproc()->my_maps->length[i] = new_size;
	  return myproc()->my_maps->addr[i];
	}
      }
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
