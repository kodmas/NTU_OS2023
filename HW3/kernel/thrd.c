#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// for mp3
uint64
sys_thrdstop(void)
{
  int delay;
  uint64 context_id_ptr;
 
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argaddr(1, &context_id_ptr) < 0)
    return -1;
  //printf("before main context_id = %d\n",context_id_ptr);
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;
  
  struct proc *proc = myproc();
  int *copy_tokernel = kalloc();
  if(copyin(proc->pagetable, (char *)copy_tokernel, context_id_ptr, sizeof(*copy_tokernel)) != 0)
    return -1;
  //printf("before main context_id = %d\n",*copy_tokernel);

  if(*copy_tokernel == -1){
    for (int i = 0; i < MAX_THRD_NUM; i++){
      if (proc->context_isused[i] == 0){
        proc->context_isused[i] = 1;
        *copy_tokernel = i;
        copyout(proc->pagetable,context_id_ptr,(char *)copy_tokernel,sizeof(*copy_tokernel));
        //printf("i= %d", i);
        break;
      }
    }
    if (context_id_ptr < 0){
      return -1;  //no more space
    }
  }
  
  proc -> totalticks = 0;
  proc -> delay_val = delay;
  proc -> context_id = *copy_tokernel;
  //printf("context_id = %d\n",proc->context_id);
  //printf("main context_id = %d\n",context_id_ptr);
  proc -> thrdstop_handler_ptr = handler;
  proc -> handler_arg = handler_arg; 
  //TODO: mp3
  
  return 0;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int context_id, is_exit;
  if (argint(0, &context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;

  if (context_id < 0 || context_id >= MAX_THRD_NUM) {
    return -1;
  }

  struct proc *proc = myproc();
  proc->context_id = context_id;
  proc->ISexit = is_exit;
  //TODO: mp3
  uint64 returnval = proc -> totalticks;
  return returnval;
}

// for mp3
uint64
sys_thrdresume(void)
{
  int context_id;
  if (argint(0, &context_id) < 0)
    return -1;

  struct proc *proc = myproc();
  proc->context_id = context_id;
  //TODO: mp3

  return 0;
}
