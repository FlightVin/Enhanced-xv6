# REPORT
# 2021101058, 2021101103

## Specification 1
### System call 1 : `trace`  
`int trace(int traceNum)`

To trace system calls only for a set of processes, we need to know if we need to trace for the currently running process. Thus we add `traceOpt` to `struct proc`.  
This variable holds 0 by default to indicate that no systemcalls are to be traced. To trace the `i`th system call, the `i`th bit is turned on (`1 << i` is added).  
Every child of a process inherits its parent's `traceOpt`. This is achieved by copying the value of `traceOpt` to the new process when forking.  
When a process invokes a system call, it calls the `syscall` function. In this function we check if tracing is on for the process and if the system call (whose syscall number defined in `syscall.h` is stored in the variable `num`) is to be traced using bit operations. If tracing is on for this system call for this process, we get the pid of the process, the syscall name (from the `syscallName` array), the arguments(using `argint` function, the argument count is stored in the array `syscallArgC`) and the return value of the syscall(from `p->trapframe->a0`).  
When a process is freed in `freeproc`, `traceOpt` is reset to 0.  
**Limitation**: The variable `traceOpt` is a signed integer(usually 32 bits long) since xv6 only provides the use of `argraw` within `sysproc.c`. Thus only the first 31 syscalls can be reliably traced. To get around this issue, `traceOpt` can be declared as `uint64`. This requires the addition of the declaration of `argraw` to `defs.h` for `syscall.c` and the replacement of `argint` calls with `argraw` where necessary. This has not been done to maintain the abstraction provided by `argint`,  `argaddr` and `argfd` and since it is quite unlikely that there will be an addition of so many system calls that will need to be traced.  

## Specification 2  
### FCFS(First Come First Serve)  
Enable with : `make clean; make qemu SCHEDULER=FCFS`  
First off, we need to modify the makefile to declare a macro according to the SCHEDULER option given when compiling. The makefile defines FCFS to 1 in every file it compiles, so now we knkow when to use FCFS.  
To select the first process to serve it, we need to know the time when the process was created. This is measured in ticks and is stored in `uint creation_time` variable in `struct proc` in `proc.h`. Next, in `proc.c`, when a process is allocated, we define its creation time to be the current ticks.  
In the function `scheduler`, we check if the FCFS macro is defined. We keep track of the earliest created process by iterating through the process table, acquiring and releasing locks as necessary(release lock if it is not going to be run(keep the lock of the earliest process)). On finding the earliest created process, we run it.  
Preemption is disabled by checking if FCFS is defined before calling `yield` to give up CPU time in `trap.c`.  

### Lottery Based Scheduling
Enable with : `make clean; make qemu SCHEDULER=LBS`  
Add `int tickets` variable to `struct proc` in `proc.h` to keep track of the number of tickets a process has.  
Implemented the `settickets` system call that changes the number of tickets of the current process.  
Initialise tickets of the process to 1 in `allocproc` in `proc.c`.  
In `schedule` in `proc.c` if `LBS` is defined, find the total number of tickets currently held by processes that are `RUNNABLE`. Randomly pick a winning ticket and find the process with the winning ticket by again iterating over all processes and run it. 

### Priority Based Scheduling
Enable with : `make clean; make qemu SCHEDULER=PBS`  
Added variables `run_time`, `sleep_time`, `num_scheduled`, `static_priority` and `niceness` to `struct proc` in `proc.c`.  
Initialise default values of 0, 0, 60, 5 respectively in `allocproc`.  
Implemented `set_priority` system call to change the static priority of a process with given pid. Also made user program `setpriority` that calls the system call with command line arguments.  
`run_time` and `sleep_time` variables are updated whenever a clock interrupt occurs.  
`niceness` is updated whenever dynamic priority is calculated or when static priority of a process is changed with `set_priority`.  
`set_priority` resets `run_time` and `sleep_time` to 0 and `niceness` to 5. If the new static priority of a process is higher than before (numerically lower), then the process yields for rescheduling.  
In `scheduler` function in `proc.c`, created variables to track the process with highest priority(numerically lowest dynamic priority value) and its dynamic priority. Made an array `cases` with the case and tiebreakers when the highest priority process must be changed to the current process. Check if any of the entries in `cases` is true to update highest priority process. On finding the highest priority process, run it and increment `num_scheduled`.    
Since it is non preemptive, turn off yielding in `trap.c`.  

## Analysis  
RR : Avg rtime 120, avg wtime 19  
FCFS :  Avg rtime 72, avg time 61  
LBS : Average rtime 127,  wtime 12  
PBS : Average rtime 109,  wtime 24