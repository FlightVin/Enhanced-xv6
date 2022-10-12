#include "proc.h"
#include "param.h"

struct _mlfq_queue{
    struct proc* proc_queues[5][NPROC];     // the multilevel queues
    int proc_queue_size[5];                 // number of processes in the queue
    int proc_queue_max_allowable_ticks[5];  // the maximum number of ticks before preemption
};

void mlfq_queue_initialization(void); // initialize s with 0s

void enque(int, struct proc*); // adds the process to the end of given queue

void remove_from_between(int, struct proc*); // removes a given process from a queue

struct proc* deque(int); // removes and returns the first element of given queue

extern struct _mlfq_queue mlfq_queue;