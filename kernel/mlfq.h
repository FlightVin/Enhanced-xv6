// structure

struct _mlfq_queue{
    struct proc* proc_queues[5][NPROC];     // the multilevel queues
    int proc_queue_size[5];                 // number of processes in the queue
    int proc_queue_max_allowable_ticks[5];  // the maximum number of ticks before preemption
};

extern struct _mlfq_queue mlfq_queue;

#define AGETIME 30