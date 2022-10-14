// inclusions are same as all other files
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#include "mlfq.h"
#define MLFQTEST 0

struct _mlfq_queue mlfq_queue;

void mlfq_queue_initialization(){
    for (int i = 0; i<5; i++){
        mlfq_queue.proc_queue_max_allowable_ticks[i] = 1 << i;
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
    mlfq_queue.proc_queues[queue_number][mlfq_queue.proc_queue_size[queue_number]++] = p;
    
    #ifdef MLFQTEST
    printf("((pid:%d ticks:%d queuenum:%d))\n", p->pid, ticks, p->queue_num);
    #endif

    p->wait_time = 0;
    p->queue_num = queue_number;
    p->is_in_queue = 1;
    p->curr_run_time = 0;
}

// returns the process itself if found in queue
// returns 0 otherwise
struct proc* remove_from_between(int queue_number, struct proc* p){
    // checking if queue's size if valid
    if (mlfq_queue.proc_queue_size[queue_number] <= 0 || mlfq_queue.proc_queue_size[queue_number] >= NPROC){
        panic("mlfq remove from between");
    }

    for (int proc_ind = 0; proc_ind < NPROC ; proc_ind++){
        if (mlfq_queue.proc_queues[queue_number][proc_ind] == p){
            
            // shifting everythig one to left
            for (int next_ind =  proc_ind; next_ind < NPROC - 1; next_ind++){
                mlfq_queue.proc_queues[queue_number][next_ind] = mlfq_queue.proc_queues[queue_number][next_ind+1]; 
            }

            // last element is 0 now
            mlfq_queue.proc_queues[queue_number][NPROC - 1] = 0;

            // size of queue has reduced by 1 now
            mlfq_queue.proc_queue_size[queue_number]--;

            // the process is no longer in a queue
            p->is_in_queue = 0;

            return p;
        }
    }

    return 0;
}


// returns the front of the queue, if the queue is not empty
// panics otherwise and returns 0
struct proc* deque(int queue_number){
    // checking if queue's size if valid
    if (mlfq_queue.proc_queue_size[queue_number] <= 0 || mlfq_queue.proc_queue_size[queue_number] >= NPROC){
        panic("mlfq deque");
        return 0;
    }

    // accessing front of queue
    struct proc* p = mlfq_queue.proc_queues[queue_number][0];

    // process is no longer in queue
    p->is_in_queue = 0;

    for (int proc_ind = 0; proc_ind < 1 ; proc_ind++){
        if (mlfq_queue.proc_queues[queue_number][proc_ind] == p){
            
            // shifting everythig one to left
            for (int next_ind =  proc_ind; next_ind < NPROC - 1; next_ind++){
                mlfq_queue.proc_queues[queue_number][next_ind] = mlfq_queue.proc_queues[queue_number][next_ind+1]; 
            }

            // last element is 0 now
            mlfq_queue.proc_queues[queue_number][NPROC - 1] = 0;

            // size of queue has reduced by 1 now
            mlfq_queue.proc_queue_size[queue_number]--;

            // the process is no longer in a queue
            p->is_in_queue = 0;

            p->curr_run_time = 0;
            
            return p;
        }
    }    

    return p;   
}