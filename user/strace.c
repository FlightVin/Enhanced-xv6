// since the mask aargument is interpreted as a signed integer(due to argint usage, refer Report), only tracing of the first 31 syscalls are supported 
#include "../kernel/types.h"
#include "../kernel/syscall.h"
#include "user.h"

int main(int argc, char **argv)
{
    // minimum of 3 arguments needed : strace, mask and command(>=1)
    if (argc < 3)
    {
        printf("Too few commands! Usage : strace <mask> <command>\n");
        exit(1);
    }

    // get trace number from commandline argument
    // if traceNum is negative or zero, it is an invalid option (since they are of form 1 << i where i is 1..num_of_syscalls)
    int traceNum = atoi(argv[1]);
    if (traceNum <= 0)
    {
        printf("Mask error! Expected positive integer. Usage : strace <mask> <command>\n");
        exit(2);
    }

    trace(traceNum); // trace is guaranteed to succeed since we check for negative traceNum

    exec(argv[2], &argv[2]); // execute the command

    //! program does not return here, do not add more code
    exit(0);
}