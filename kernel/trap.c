#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "mlfq.h"

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void
usertrap(void)
{
  int which_dev = 0;

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();
  
  // save user program counter.
  p->trapframe->epc = r_sepc();
  
 if (r_scause() == 15){
    // page fault

    // get lower page aligned address
    uint64 va = PGROUNDDOWN(r_stval());

    if (va >= MAXVA || va == 0)
      p->killed = 1;
    else {

      pte_t* pte = walk(p->pagetable, va, 0);

      if (pte != 0){
        uint flags = PTE_FLAGS(*pte);
        // check validity of PTE
        if ((flags & PTE_V) != 0){
          // check whether COW
          if ((flags & PTE_COW) != 0){
            
            // add write flag
            flags = flags | PTE_W;

            // remove COW flag
            flags = flags & (~PTE_COW);

            void* mem = kalloc();
            if (mem == 0){
              p->killed = 1;
            } else {
              // get phsical address
              uint64 pa = PTE2PA(*pte);

              // updating PTE
              // mapping physial address - 
              *pte = PA2PTE(mem);
              // adding new flags
              *pte = *pte | flags;

              // copy information from pa to the new memory space
              memmove(mem, (void*)pa, PGSIZE);

              // freeing the page of memory pointed by pa, i.e., 
              kfree((void*)pa);
            }

          } else {
            p->killed = 1;
          }
        } else {
          p->killed = 1;
        }
      } else {
        p->killed = 1;
      }
    }

  } else if(r_scause() == 8){
    // system call

    if(killed(p))
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sepc, scause, and sstatus,
    // so enable only now that we're done with those registers.
    intr_on();

    syscall();
  } else if((which_dev = devintr()) != 0){
    
    // 2 -> timer interrupt = one tick
    if (which_dev == 2){
      p->current_ticks_count++;
      if (p->sigalarm_ticks > 0){ // if we are even checking for ticks
        if (p->sigalarm_en == 0){ // when we aren't in a sigalarm sequence
          if (p->current_ticks_count >= p->sigalarm_ticks){
            // we are now in a sigalarm sequence
            p->sigalarm_en = 1;

            // initialize ticks count back to 0
            p->current_ticks_count = 0;

            // creating backup
            p->tm_backup = (struct trapframe*) kalloc();
            memmove(p->tm_backup, p->trapframe, sizeof(struct trapframe));

            // handling handler function by changing program counter
            p->trapframe->epc = p->sig_handler;
          }
        }
      }
#ifndef FCFS
#ifndef PBS
#ifndef MLFQ
      yield(); // disable preemption if FCFS is selected
#endif
#endif
#endif

#ifdef MLFQ
      if (p && p->state == RUNNING)
      {
        p->curr_run_time++;

        for (int i = 0; i < p->queue_num; i++)
        {
          if (mlfq_queue.proc_queue_size[i] != 0)
          {
            enque(p->queue_num, p);
            yield();
          }
        }

        if (p->curr_run_time >= mlfq_queue.proc_queue_max_allowable_ticks[p->queue_num])
        {
          // remove_from_between(p->queue_num, p);
          enque(4 < p->queue_num + 1 ? 4 : p->queue_num + 1, p);
          yield();
        }
      }
#endif
    }

  } else {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    setkilled(p);
  }

  if(killed(p))
    exit(-1);

  usertrapret();
}

//
// return to user space
//
void
usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to uservec in trampoline.S
  uint64 trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
  w_stvec(trampoline_uservec);

  // set up trapframe values that uservec will need when
  // the process next traps into the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.
  
  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to userret in trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64))trampoline_userret)(satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
#ifndef FCFS
#ifndef PBS
#ifndef MLFQ
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield(); // disable preemption if FCFS is selected
#endif
#endif
#endif

#ifdef MLFQ
      struct proc *p = myproc();
      if (p)
      {
        if (p->state == RUNNING)
          p->curr_run_time++;

        for (int i = 0; i < p->queue_num; i++)
        {
          if (mlfq_queue.proc_queue_size[i] != 0)
          {
            enque(p->queue_num, p);
            yield();
          }
        }
        
        if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING && p->curr_run_time >= mlfq_queue.proc_queue_max_allowable_ticks[p->queue_num])
        {
          // deque(p->queue_num);
          // remove_from_between(p->queue_num, p);
          enque(4 < p->queue_num + 1 ? 4 : p->queue_num + 1, p);
          yield();
        }
      }
#endif

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
clockintr()
{
  acquire(&tickslock);
  ticks++;
  update_time();
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if((scause & 0x8000000000000000L) &&
     (scause & 0xff) == 9){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    } else if(irq){
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq);

    return 1;
  } else if(scause == 0x8000000000000001L){
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if(cpuid() == 0){
      clockintr();
    }
    
    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  } else {
    return 0;
  }
}

