#include "../kernel/types.h"
#include "../kernel/syscall.h"
#include "user.h"

int main(int argc, char **argv)
{
    // 3 arguments needed : setpriority, priority and pid
    if (argc != 3)
    {
        printf("Usage : setpriority priority pid\n");
        exit(1);
    }

    int new_priority = atoi(argv[1]);
    int pid = atoi(argv[2]);
    
    set_priority(new_priority, pid);
    
    exit(0);
}