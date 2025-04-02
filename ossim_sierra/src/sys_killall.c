/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"

// fix compiler warning
#include "queue.h"

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1; // id of memregion 
    
    /* TODO: Get name of the target proc */
    //proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    /* TODO: Traverse proclist to terminate the proc
     *       strcmp to check the process match proc_name
     */
    //caller->running_list
    //caller->mlq_ready_queue

    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */

    struct pcb_t *proc = NULL;

    // running process list 
    struct queue_t *runnlist = caller->running_list;
    int j = 0;
    for(; j < runnlist->size; j++){
        proc = dequeue(runnlist);

        // so sanh ten tien trinh vua lay ra 
        if (strcmp(proc->path, proc_name) == 0) {
            printf("Terminating process PID=%d with name \"%s\"\n", proc->pid, proc->path);
            free_pcb_memph(proc); // Giải phóng bộ nhớ của tiến trình
            free(proc->code->text); // Giải phóng mã lệnh của tiến trình
            free(proc); // Giải phóng cấu trúc PCB
        } else {
            enqueue(runnlist, proc); // Đưa tiến trình trở lại hàng đợi nếu không khớp
        }
    }
    #ifdef MLQ_SCHED
    // MLQ policy 

    // Duyệt qua các tiến trình trong hàng đợi MLQ
struct queue_t *queue = caller->mlq_ready_queue;
int queue_size = queue->size; // Lưu kích thước ban đầu của hàng đợi

for (int j = 0; j < queue_size; j++) {
    struct pcb_t *proc = dequeue(queue);
    if (proc == NULL) continue;

    if (strcmp(proc->path, proc_name) == 0) {
        
        free_pcb_memph(proc); // Giải phóng bộ nhớ của tiến trình
        if (proc->code && proc->code->text) {
            free(proc->code->text); // Giải phóng mã lệnh của tiến trình
        }
        free(proc); // Giải phóng cấu trúc PCB
    } else {
        enqueue(queue, proc); // Đưa tiến trình trở lại hàng đợi nếu không khớp
    }
}
#endif


    

    return 0; 
}
