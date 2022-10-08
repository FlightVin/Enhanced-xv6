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