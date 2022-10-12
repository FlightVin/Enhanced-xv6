#include "mlfq.h"

struct _mlfq_queue mlfq_queue;

void mlfq_queue_initialization(){
    for (int i = 0; i<5; i++){
        mlfq_queue.proc_queue_size[i] = 1 << i;
        mlfq_queue.proc_queue_size[i] = 0;
        
        // initially there are no processes in any queue
        for (int j = 0; j<NPROC; j++){
            mlfq_queue.proc_queues[i][j] = 0;
        }
    }
}

void enque(int queue_number, struct proc* p){
    // checking if queue's size if valid
    if (mlfq_queue.proc_queue_size[queue_number] < 0 || mlfq_queue.proc_queue_size[queue_number] >= NPROC - 1){
        panic("mlfq enque");
    }

    // initializations
    
}