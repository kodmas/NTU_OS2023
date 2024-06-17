#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault(uint specific_va) {
  uint64 va;
  /* Find the address that caused the fault */
  if(specific_va == -1) va = r_stval(); 
  else va = specific_va;

  /* TODO */
  struct proc *p = myproc();
  pte_t *pte = walk(p->pagetable,va,0);
  if (*pte & PTE_S){ 
    uint64 block_no = PTE2BLOCKNO(*pte); //obtain block no
    *pte |= PTE_V; //set PTE_V
    *pte &= ~PTE_S; //unset PTE_S
    void* physical_page = kalloc(); //allocate
    begin_op();
    read_page_from_disk(ROOTDEV,(char*)physical_page,block_no);
    bfree_page(ROOTDEV,block_no);
    end_op();
    *pte = PA2PTE(physical_page) | PTE_FLAGS(*pte);
  }
  char *mem = kalloc();
  if (mem != 0){ //legal
    memset(mem,0,PGSIZE);
    int mappage_success = mappages(p->pagetable,PGROUNDDOWN(va),PGSIZE,(uint64)mem,PTE_U|PTE_R|PTE_W|PTE_X);
    if (mappage_success != 0){
        kfree(mem);
        p->killed = 1;
    }
  }
  else{
    p->killed = 1;
  }
  return 0;
  ///panic("not implemented yet\n");
}
