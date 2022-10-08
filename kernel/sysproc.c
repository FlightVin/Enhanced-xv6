#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// edit the processes' trace option
uint64
sys_trace(void)
{
  // fetch the trace number to be applied from a0
  int traceNum = 0;
  argint(0, &traceNum);

  if (traceNum < 0)
  {
    // invalid, must be non negative
    return -1;
  }

  // apply the trace number as the trace option for current process
  struct proc *currProc = myproc();
  currProc->traceOpt = traceNum;

  return 0;
}

// set an alarm to executing handler
uint64
sys_sigalarm(void)
{
  // retrieve arguments

  // printf("called alarm\n");

  int sigalarm_ticks;
  uint64 sigalarm_handler;

  argaddr(1, &sigalarm_handler);
  argint(0, &sigalarm_ticks);

  if (sigalarm_ticks == -1 || sigalarm_handler == -1)
    return -1;

  struct proc *p = myproc();

  p->sigalarm_ticks = sigalarm_ticks;
  p->sig_handler = sigalarm_handler;
  p->sigalarm_en = 0;
  p->current_ticks_count = 0;

  return 0;
}

// reset process state
uint64
sys_sigreturn(void)
{

  // printf("called return\n");

  struct proc *p = myproc();

  // backup restoration for test1 and test2
  memmove(p->trapframe, p->tm_backup, sizeof(struct trapframe));
  if (p->tm_backup)
    kfree(p->tm_backup);
  p->tm_backup = 0;

  p->sigalarm_en = 0;

  return p->trapframe->a0; // to restore a0
}
